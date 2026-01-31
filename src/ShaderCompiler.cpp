// ╒═════════════════ ShaderCompiler.cpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-01-30                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// │ Placeholder License                  │
// ╰──────────────────────────────────────╯

#include "ShaderCompiler.hpp"

#include <cstdlib>
#include <iostream>

namespace SynTools {
bool ShaderCompiler::CompileShader(std::string                     fragmentPath,
                                   std::string                     vertexPath,
                                   const std::string&              outputPath,
                                   const std::vector<std::string>& options) {
    // Decode options and set up compilation parameters
#ifdef _WIN32
    std::string compilerPath = "shaderc"; // Default shader compiler path
    std::string platform     = "windows";
#else
    std::string compilerPath = "./shaderc"; // Default shader compiler path
#ifdef __APPLE__
    std::string platform = "metal";
#else
    std::string platform = "spirv";
#endif
#endif
    std::string srcDir = "../../assets/shaders/";
    std::string varying      = vertexPath + ".vary";
    std::string srcExtension = ".sc";
    std::string outExtension = ".sc.bin";
    std::string includes = "-i ../../engine/third_party/bgfx.cmake/bgfx/src/";
    // Frankly, I don't care about code cleanliness here - this is a tool (A bit like myself, but anyway...)
    for (const auto& option : options) {
        if (option.rfind("--compiler=", 0) == 0) {
            compilerPath = option.substr(11);
        } /*else if (option.rfind("--platform=", 0) == 0) { // Only allow platform to be set by OS currently
            platform = option.substr(11);
        }*/ else if (option.rfind("--varying=", 0) == 0) {
            varying = option.substr(10);
        } else if (option.rfind("--src-ext=", 0) == 0) {
            srcExtension = option.substr(10);
        } else if (option.rfind("--out-ext=", 0) == 0) {
            outExtension = option.substr(10);
        } else if (option.rfind("--src-dir=", 0) == 0) {
            srcDir = option.substr(10);
        } else if (option.rfind("--include=", 0) == 0) {
            includes += " -i " + option.substr(10);
        }
    }

    // Compile vertex shader
    std::string vertCommand = compilerPath + " -f " + srcDir + vertexPath +
                              ".vert" + srcExtension + " -o " + outputPath +
                              outExtension + " -p " + platform + " --type v" +
                              " --varyingdef " + srcDir + "varying/" + varying +
                              srcExtension + " " + includes;

    std::cout << vertCommand << std::endl;
    
    int vertResult = std::system(vertCommand.c_str());
    if (vertResult != 0) {
        return false;
    }
    
    return true;
}

void ShaderCompiler::PrintHelp() {
    std::cout << "ShaderCompiler - Compile shaders for Syngine\n";
    std::cout << "Usage: ShaderCompiler <fragment_shader> <vertex_shader> "
                 "<output_path> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --compiler=<path>     Path to shader compiler executable "
                 "(default: shaderc)\n";
    std::cout << "  --platform=<platform> Target platform (default: dx12)\n";
    std::cout << "  --varying=<path>      Path to varying definitions file\n";
    std::cout << "  --src-ext=<ext>       Source file extension (default: .sc)\n";
    std::cout
        << "  --out-ext=<ext>       Output file extension (default: .sc.bin)\n";
    std::cout << "  --src-dir=<dir>       Source directory for shaders\n";
    std::cout << "  --include=<dir>       Additional include directory (Multiple allowed)\n";
}
} // namespace SynTools
