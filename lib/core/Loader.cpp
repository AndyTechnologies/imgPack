#include "Loader.hpp"
#include <stb.hpp>
#include <zstdpp.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <print>
#include <stdexcept>

namespace imgp {

namespace fs = std::filesystem;

static constexpr uint8_t TEXTURE_ATLAS_VERSION_BINARIES{2u};

TextureAtlas::TextureAtlas(const TextureAtlas::str_t &binaryPath)
    : atlasSize(0), pad(0) {
  if (!fs::exists(binaryPath)) {
    atlasSize = 512;
    pad = 1;
    context = new stbrp_context();
    return;
  }

  // Reutilizamos el loader estático:
  *this = TextureAtlas::loadBinary(binaryPath);

  // Como loadBinary asigna atlasSize y pixels y regions,
  // necesitamos inicializar context para posibles rebuilds:
  context = new stbrp_context;
}

TextureAtlas::TextureAtlas(TextureAtlas::int_t initialSize,
                           TextureAtlas::int_t padding)
    : atlasSize(initialSize), pad(padding) {
  context = new stbrp_context();
}

TextureAtlas::~TextureAtlas() { delete context; }

TextureAtlas::vec8_t TextureAtlas::build(const TextureAtlas::vec_str_t &files) {
  loadImages(files);
  packRects();
  composeAtlas();
  return pixels;
}

TextureAtlas::vec8_t TextureAtlas::build(const TextureAtlas::str_t &file) {
  filenames.push_back(file);
  return build(TextureAtlas::vec_str_t{filenames});
}

bool TextureAtlas::exportPNG(const TextureAtlas::str_t &outPath) const {
  if (pixels.empty())
    throw std::runtime_error("Atlas vacío. Llame a build() primero.");
  int result = stbi_write_png(outPath.c_str(), atlasSize, atlasSize, 4,
                              pixels.data(), atlasSize * 4);
  if (!result)
    throw std::runtime_error("Error escribiendo PNG");
  return true;
}

const std::unordered_map<TextureAtlas::str_t, TextureAtlas::Region> &
TextureAtlas::getRegions() const {
  return regions;
}

int TextureAtlas::size() const { return atlasSize; }

void TextureAtlas::loadImages(const TextureAtlas::vec_str_t &files) {
  images.clear();
  filenames.clear();
  rects.clear();
  regions.clear();
  images.reserve(files.size());
  filenames.reserve(files.size());
  rects.reserve(files.size());

  for (size_t i = 0; i < files.size(); ++i) {
    int w, h, comp;
    auto path = files[i];
    uint8_t *data = stbi_load(path.c_str(), &w, &h, &comp, 4);
    if (!data)
      throw std::runtime_error("No se pudo cargar: " + path);

    images.push_back({w, h, TextureAtlas::vec8_t(data, data + w * h * 4)});
    filenames.push_back(path);
    stbi_image_free(data);

    // id, w, h, x, y, was_packed
    rects.push_back({(int)i, w + pad * 2, h + pad * 2, 0, 0, 0});
  }
}

TextureAtlas::Region
TextureAtlas::getRegionFor(TextureAtlas::str_t region_name) const {
  return regions.at(region_name);
}

void TextureAtlas::packRects() {
  static constexpr std::size_t maxSize = (INT32_MAX / INT8_MAX);
  while (true) {
    for (auto &r : rects) {
      r.x = r.y = 0;
      r.was_packed = 0;
    }
    nodes.resize(atlasSize);
    stbrp_init_target(context, atlasSize, atlasSize, nodes.data(),
                      nodes.size());
    stbrp_pack_rects(context, rects.data(), rects.size());

    bool allPacked = true;
    for (auto &r : rects)
      if (!r.was_packed) {
        allPacked = false;
        break;
      }
    if (allPacked)
      break;

    atlasSize *= 2;
    if (atlasSize > maxSize) {
      std::println("Atlas size {} (max: {})", atlasSize, maxSize);
      throw std::runtime_error("Atlas demasiado grande.");
    }
  }

  regions.clear();
  for (size_t i = 0; i < rects.size(); ++i) {
    auto &r = rects[i];
    regions[filenames[i]] = {r.x + pad, r.y + pad, r.w - pad * 2,
                             r.h - pad * 2};
  }
}

void TextureAtlas::composeAtlas() {
  pixels.assign(atlasSize * atlasSize * 4, 0);
  for (size_t i = 0; i < images.size(); ++i) {
    auto &img = images[i];
    auto &r = rects[i];
    if (!r.was_packed)
      continue;
    for (int y = 0; y < img.h; ++y) {
      for (int x = 0; x < img.w; ++x) {
        int dx = r.x + pad + x;
        int dy = r.y + pad + y;
        size_t dst = (size_t)dy * atlasSize * 4 + dx * 4;
        size_t src = (size_t)y * img.w * 4 + x * 4;
        for (int c = 0; c < 4; ++c)
          pixels[dst + c] = img.data[src + c];
      }
    }
  }
}

void TextureAtlas::exportBinary(const TextureAtlas::str_t &outPath) const {
  if (pixels.empty())
    throw std::runtime_error("Atlas vacío. Llame a build() primero.");

  std::ofstream os(outPath, std::ios::binary);
  if (!os)
    throw std::runtime_error("No se pudo abrir " + outPath);

  // Magic + versión
  os.write("TXAT", 4);
  uint8_t version = TEXTURE_ATLAS_VERSION_BINARIES;
  os.write(reinterpret_cast<char *>(&version), 1);

  // Tamaño del atlas
  int32_t sz = atlasSize;
  os.write(reinterpret_cast<char *>(&sz), sizeof(sz));

  // Número de regiones
  uint32_t n = static_cast<uint32_t>(regions.size());
  os.write(reinterpret_cast<char *>(&n), sizeof(n));

  // Regiones
  for (auto &kv : regions) {
    const TextureAtlas::str_t &name = kv.first;
    const Region &r = kv.second;

    uint16_t L = static_cast<uint16_t>(name.size());
    os.write(reinterpret_cast<char *>(&L), sizeof(L));
    os.write(name.data(), L);

    os.write(reinterpret_cast<const char *>(&r.x), sizeof(r.x));
    os.write(reinterpret_cast<const char *>(&r.y), sizeof(r.y));
    os.write(reinterpret_cast<const char *>(&r.width), sizeof(r.width));
    os.write(reinterpret_cast<const char *>(&r.height), sizeof(r.height));
  }

  auto outBuff = zstdpp::compress(pixels);
  // Cantidad de Píxeles comprimidos
  auto countPixels = outBuff.size();
  std::println("Buffer count {} of {}", countPixels, pixels.size());
  os.write(reinterpret_cast<const char *>(&countPixels), sizeof(std::size_t));
  // Pixeles comprimidos
  os.write(reinterpret_cast<const char *>(outBuff.data()), countPixels);
  if (!os)
    throw std::runtime_error("Error al escribir datos en " + outPath);
}

TextureAtlas TextureAtlas::loadBinary(const TextureAtlas::str_t &inPath) {
  std::ifstream is(inPath, std::ios::binary);
  if (!is)
    throw std::runtime_error("No se pudo abrir " + inPath);

  // Magic + versión
  char magic[4];
  is.read(magic, 4);
  if (TextureAtlas::str_t(magic, 4) != "TXAT")
    throw std::runtime_error("Formato no reconocido: " + inPath);

  uint8_t version;
  is.read(reinterpret_cast<char *>(&version), 1);
  if (version != TEXTURE_ATLAS_VERSION_BINARIES)
    throw std::runtime_error("Versión desconocida: " + std::to_string(version));

  // Tamaño del atlas
  int32_t sz;
  is.read(reinterpret_cast<char *>(&sz), sizeof(sz));
  TextureAtlas atlas(sz /* initialSize */, 0 /* padding ya aplicado */);
  atlas.atlasSize = sz;

  // Número de regiones
  uint32_t n;
  is.read(reinterpret_cast<char *>(&n), sizeof(n));

  // Leer regiones
  atlas.regions.clear();
  for (uint32_t i = 0; i < n; ++i) {
    uint16_t L;
    is.read(reinterpret_cast<char *>(&L), sizeof(L));
    TextureAtlas::str_t name(L, '\0');
    is.read(&name[0], L);

    Region r;
    is.read(reinterpret_cast<char *>(&r.x), sizeof(r.x));
    is.read(reinterpret_cast<char *>(&r.y), sizeof(r.y));
    is.read(reinterpret_cast<char *>(&r.width), sizeof(r.width));
    is.read(reinterpret_cast<char *>(&r.height), sizeof(r.height));

    atlas.regions.emplace(std::move(name), r);
  }

  // Leer píxeles
  // Cargar buffer comprimido
  zstdpp::buffer_t inBuffer{};

  // Load Compressed Buffer Size
  std::size_t sizeBuffer{};
  is.read(reinterpret_cast<char *>(&sizeBuffer), sizeof(std::size_t));

  // Load Compressed Buffer
  inBuffer.resize(sizeBuffer);
  is.read(reinterpret_cast<char *>(inBuffer.data()), sizeBuffer);
  inBuffer.shrink_to_fit();

  // Descomprimir buffer
  auto buff = zstdpp::decompress(inBuffer);
  atlas.pixels.insert(atlas.pixels.begin(), buff.begin(), buff.end());
  if (!is)
    throw std::runtime_error("Datos de píxeles incompletos");

  return atlas;
}

} // namespace imgp
