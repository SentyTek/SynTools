// ╒═════════════════ ShaderCompiler.hpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-01-30                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// | Licensed under the MIT License       |
// ╰──────────────────────────────────────╯

#pragma once

#include <string>
#include <vector>

namespace SynTools {
class ShaderCompiler {
  public:
    static bool CompileShader(std::string                     fragmentPath,
                              std::string                     vertexPath,
                              std::string                     outputPath,
                              const std::vector<std::string>& options = {});

    static void PrintHelp();
};
    
} // namespace SynTools
