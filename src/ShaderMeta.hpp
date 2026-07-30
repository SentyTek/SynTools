// ╒═════════════════════ ShaderMeta.hpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-07-28                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// │ Licensed under the MIT License       │
// ╰──────────────────────────────────────╯

#include <string>
#include <vector>
#include <filesystem>

namespace SynTools {

class ShaderMeta {
    struct _UniformMeta {
        std::string               name;
        std::string               type;
        std::string               frequency;
        std::string               num;
        std::string               src;
        std::string               defaultValue;
        std::string               texStage; // For samplers, the texture stage
        std::vector<_UniformMeta> children; // For nested structures
    };

    struct _Shader {
        std::string               name;
        std::vector<_UniformMeta> EngineUniforms;
        std::vector<_UniformMeta> EngineSamplers;
        std::vector<_UniformMeta> MaterialParams;
        std::vector<_UniformMeta> Samplers;
    };

    static void _parseFile(const std::string& content, _Shader& shader);
    static void _preprocessFile(const std::filesystem::path& filePath,
                                std::string&                 outContent,
                                bool                         verbose);

  public:
    static void PrintHelp();
    static bool GenerateShaderMeta(const std::string&              shaderPath,
                                   const std::vector<std::string>& options);
};

} // namespace SynTools
