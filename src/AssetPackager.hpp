// ╒══════════════════ AssetPackager.hpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-01-30                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// │ Licensed under the MIT License       │
// ╰──────────────────────────────────────╯

#pragma once

#include <string>
#include <vector>

namespace SynTools {
class AssetPackager {
  public:
    AssetPackager();
    ~AssetPackager();
    bool CreateFileBundle(const std::vector<std::string>& files,
                          const std::string&              outputPath,
                          const std::string&              execPath = "",
                          const std::vector<std::string>& options  = {});

    void PrintHelpPack();
    void PrintHelpTree();
    void PrintHelpShaders();

    void ValidatePackage(const std::string& packagePath);

    void ViewAsset(const std::string& assetPath, const std::string& asset);

    bool PackTree(const std::string&              rootDir,
                  const std::string&              outputPath,
                  const std::vector<std::string>& options = {});
};
}; // namespace SynTools
