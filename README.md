
# Image Packager

This library aims to provide a way to load/save images efficiently.

> [!CAUTION]
> Do not use this for production-ready software; it is in a very early stage of development and may contain many bugs and security risks.

## Usage

```cpp
// Init void atlas (if exist a file "texture_atlas.bin", this autoload that content)
imgp::TextureAtlas atlas{};
std::vector<std::string> files = {"textures/icon.png", "textures/hero.png", "textures/bg.png"};

// Build file per file
for (auto &f : files)
 atlas.build(f);
// or all files
atlas.build(files);

// Export to the internal format binary
atlas.exportBinary(); // you can specify another name for the binary output. (default: "texture_atlas.bin")

// Create new texture atlas for auto-load the exported binary ("texture_atlas.bin" by default)
imgp::TextureAtlas atlas2{};
atlas2.exportPNG("output_atlas.png");
```
