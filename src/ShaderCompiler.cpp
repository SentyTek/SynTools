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
#include <vector>

namespace SynTools {
bool ShaderCompiler::CompileShader(std::string                     fragmentPath,
                                   std::string                     vertexPath,
                                   std::string                     outputPath,
                                   const std::vector<std::string>& options) {
    // Decode options and set up compilation parameters
#ifdef _WIN32
    std::string compilerPath = "shaderc"; // Default shader compiler path
    std::string platform     = "s_5_0"; // DX 12 is HLSL 5
#else
    std::string compilerPath = "./shaderc"; // Default shader compiler path
#ifdef __APPLE__
    std::string platform = "metal";
#else
    std::string platform = "spirv";
#endif
#endif
    std::string srcDir = "../../assets/shaders/";
    std::string varying      = fragmentPath + ".vary";
    std::string srcExtension = ".sc";
    std::string outExtension = ".bin";
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

    // Ensure srcDir ends with a slash
    if (srcDir.back() != '/' && srcDir.back() != '\\') {
        srcDir += '/';
    }

    // If supplied vert / output are "s", assume frag is the name for vert and
    // output files. This is just a convenience feature, allows users to have a
    // command more like "shader myshader s s" to compile both shaders named
    // "myshader"
    if (vertexPath == "s") {
        vertexPath = fragmentPath;
    }
    if (outputPath == "s") {
        outputPath = fragmentPath;
    }

    // Compile vertex shader
    std::string vertCommand =
        compilerPath + " -f " + srcDir + vertexPath + ".vert" + srcExtension +
        " -o " + outputPath + ".vert" + outExtension + " -p " + platform +
        " --type v" + " --varyingdef " + srcDir + "varying/" + varying +
        srcExtension + " " + includes;

    std::cout << "Compiling vertex shader with command: " << vertCommand << std::endl;
    int vertResult = std::system(vertCommand.c_str());
    if (vertResult != 0) {
        return false;
    }

    // Compile fragment shader
    std::string fragCommand =
        compilerPath + " -f " + srcDir + fragmentPath + ".frag" + srcExtension +
        " -o " + outputPath + ".frag" + outExtension + " -p " + platform +
        " --type f" + " --varyingdef " + srcDir + "varying/" + varying +
        srcExtension + " " + includes;
    
    std::cout << "Compiling fragment shader with command: " << fragCommand << std::endl;
    int fragResult = std::system(fragCommand.c_str());
    if (fragResult != 0) {
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
    //std::cout << "  --platform=<platform> Target platform (default: dx12 on win32, metal on macOS, spirv on others)\n";
    std::cout << "  --varying=<path>      Path to varying definitions file\n";
    std::cout << "  --src-ext=<ext>       Source file extension (default: .sc)\n";
    std::cout
        << "  --out-ext=<ext>       Output file extension (default: .sc.bin)\n";
    std::cout << "  --src-dir=<dir>       Source directory for shaders\n";
    std::cout << "  --include=<dir>       Additional include directory "
                 "(Multiple allowed)\n";
    std::cout << "\nExamples:\n";
    std::cout << "syntools shader space s s\n";
    std::cout
        << "\tCompiles 'space.vert.sc' and 'space.frag.sc' from the default "
           "shader directory (root/assets/shaders) into respective bin "
           "files.\n";
    std::cout << "syntools shader screen s screen_output --out-ext=.shader --src-ext=.shadercode\n";
    std::cout
        << "\tCompiles 'screen.vert.shadercode' and 'screen.frag.shadercode' "
           "into 'screen_output.vert.shader' and "
           "'screen_output.frag.shader'.\n";
}
} // namespace SynTools
