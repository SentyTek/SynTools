// ╒═════════════════ ShaderCompiler.hpp ═╕
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
class ShaderCompiler {
  public:
    static bool CompileShader(std::string                     fragmentPath,
                              std::string                     vertexPath,
                              const std::string&              outputPath,
                              const std::vector<std::string>& options = {});

    static void PrintHelp();
};
    
} // namespace SynTools
