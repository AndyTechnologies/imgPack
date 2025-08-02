#include <imgpacker.hpp>
#include <print>

int main() {
  std::println("Init Atlas...");
  imgp::TextureAtlas atlas{};
  std::println("Setting up files...");
  std::vector<std::string> files = {"textures/icon.png", "textures/hero.png",
                                    "textures/bg.png"};

  // Crear y exportar atlas
  std::println("Building...");
  for (auto &f : files)
    atlas.build(f);
  std::println("Exporting...");
  atlas.exportBinary();

  std::println("Reloading atlas...");
  imgp::TextureAtlas atlas2{};
  std::println("Exporting to PNG...");
  atlas2.exportPNG("output_atlas.png");

  return 0;
}
