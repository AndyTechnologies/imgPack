#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations of stb_rect_pack types
struct stbrp_context;
struct stbrp_node;
struct stbrp_rect;

namespace imgp {

class TextureAtlas {
public:
  // Basic aliases
  using byte_t = std::uint8_t;
  using int_t = std::int32_t;
  using str_t = std::string;

  // Region struct
  struct Region {
    int_t x, y, width, height;
  };

  // Vectors aliases
  using vec8_t = std::vector<byte_t>;
  using vec_str_t = std::vector<str_t>;

  // Map aliases
  using regions_map_t = std::unordered_map<str_t, Region>;

  // Nuevo constructor: intenta cargar binario
  // si el fichero no existe, lanza excepción
  explicit TextureAtlas(const str_t &binaryPath = "texture_atlas.bin");

  explicit TextureAtlas(int_t initialSize, int_t padding = 2);
  ~TextureAtlas();

  // Build atlas from file paths
  vec8_t build(const vec_str_t &files);
  vec8_t build(const str_t &file);

  bool exportPNG(const str_t &outPath) const;

  Region getRegionFor(str_t region_name) const;
  const std::unordered_map<str_t, Region> &getRegions() const;
  int_t size() const;

  template <typename Self> auto &&getPixels(this Self &&self) {
    return std::forward<Self>(self).pixels;
  }

  // Serialización binaria
  void exportBinary(const str_t &outPath = "texture_atlas.bin") const;
  static TextureAtlas loadBinary(const str_t &inPath);

private:
  // Load image data and setup rectangles
  void loadImages(const vec_str_t &files);
  // Pack rectangles using stb_rect_pack
  void packRects();
  // Compose final pixel buffer
  void composeAtlas();

  int_t atlasSize;
  int_t pad;
  vec8_t pixels;

  vec_str_t filenames;
  regions_map_t regions;

  struct Img {
    int_t w, h;
    vec8_t data;
  };
  std::vector<Img> images;

  // stb_rect_pack members
  stbrp_context *context;
  std::vector<stbrp_node> nodes;
  std::vector<stbrp_rect> rects;
};

}; // namespace imgp
