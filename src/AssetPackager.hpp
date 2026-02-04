// ╒══════════════════ AssetPackager.hpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-01-30                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// │ Placeholder License                  │
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

    void PrintHelp();

    void ValidatePackage(const std::string& packagePath);
};
}; // namespace SynTools
