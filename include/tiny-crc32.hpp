#pragma once
#include <cstdint>

namespace tcrc32{

	namespace _internals{
		inline constexpr std::uint32_t __crc32_impl(
			const uint8_t* data, 
			std::size_t length, 
			std::uint32_t init = 0
		);
	}

inline constexpr std::uint32_t crc32(const uint8_t* data, std::size_t length, std::uint32_t init = 0);

// Overload for container with data() and size()
template <typename Container>
concept STLContainer = requires(Container a) { a.data(); a.size(); };

template<STLContainer Container>
inline constexpr std::uint32_t crc32(const Container& c, std::uint32_t init = 0) {
	return crc32(reinterpret_cast<const uint8_t*>(c.data()), c.size(), init);
}

// Incremental CRC32C
class CRC32C {
public:
    constexpr CRC32C(std::uint32_t init = 0);

    inline constexpr void update(const uint8_t* data, std::size_t length);

    template<STLContainer Container>
    inline constexpr void update(const Container& c) {
        update(reinterpret_cast<const uint8_t*>(c.data()), c.size());
    }

    [[nodiscard]] inline constexpr std::uint32_t digest() const noexcept;

private:
    std::uint32_t crc_;
};

}

#ifdef _IMPL_TINY_CRC32_

#include <memory>
#include <array>

namespace tcrc32{
	namespace _internals{
		static constexpr const std::uint32_t POLY{  0x82f63b78u };

		static consteval std::array<std::uint32_t, 256> compute() {
			std::array<std::uint32_t, 256> table{};
			for (std::uint32_t i = 0; i < 256; ++i) {
				std::uint32_t crc = i;
				for (int j = 0; j < 8; ++j) {
					if (crc & 1u)
						crc = (crc >> 1) ^ POLY;
					else
						crc >>= 1;
				}
				table[i] = crc;
			}
			return table;
		}
		static constexpr auto CRC_TABLE{ compute() };

		inline constexpr std::uint32_t __crc32_impl(
			const uint8_t* data, 
			std::size_t length, 
			std::uint32_t init
		) {
			std::uint32_t crc = ~init;
			for (std::size_t i = 0; i < length; ++i) {
				uint8_t byte = data[i];
				crc = CRC_TABLE[(crc ^ byte) & 0xFFu] ^ (crc >> 8);
			}
			return ~crc;
		}


	}

	inline constexpr std::uint32_t crc32(
		const uint8_t* data, 
		std::size_t length, 
		std::uint32_t init
	){ return _internals::__crc32_impl(data, length, init); }

	constexpr CRC32C::CRC32C(std::uint32_t init): crc_(~init){}
	
	inline constexpr void CRC32C::update(const uint8_t* data, std::size_t length){
		for (std::size_t i = 0; i < length; ++i) {
            crc_ = _internals::CRC_TABLE[(crc_ ^ data[i]) & 0xFFu] ^ (crc_ >> 8);
        }
	}

	inline constexpr std::uint32_t CRC32C::digest() const noexcept { return ~crc_; }


}
#endif