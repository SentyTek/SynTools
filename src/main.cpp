// ╒═══════════════════════════ main.cpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-01-30                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// │ Placeholder License                  │
// ╰──────────────────────────────────────╯

#define SYNGINE_TOOL_VERSION "1"

#include "AssetPackager.hpp"
#include "ShaderCompiler.hpp"

#include <iostream>

void printHelp() {
    std::cout << "Syngine Tools - Command Line Utility\n";
    std::cout << "Usage: syngine_tools <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  shader       Compile shaders\n";
    std::cout << "  package      Package game assets\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --help, -h        Show this help message\n";
    std::cout << "  --version, -v     Show tool version\n";
}

int main(int argc, char** argv) {
    // Basic arguments
    if (argc < 2 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        printHelp();
        return 0;
    }

    if (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v") {
        std::cout << "Syngine Tools Version: " << SYNGINE_TOOL_VERSION << std::endl;
        return 0;
    }

    std::string command = argv[1];
    if (command == "shader") {
        if (argc < 5) {
            SynTools::ShaderCompiler::PrintHelp();
            return 1;
        }
        std::string vertexPath   = argv[2];
        std::string fragmentPath = argv[3];
        std::string outputPath   = argv[4];
        std::vector<std::string> options;
        for (int i = 5; i < argc; ++i) {
            options.push_back(argv[i]);
        }
        bool success = SynTools::ShaderCompiler::CompileShader(fragmentPath, vertexPath, outputPath, options);
        if (!success) {
            std::cerr << "Shader compilation failed." << std::endl;
            return 1;
        }
        std::cout << "Shader compiled successfully." << std::endl;
    }
    
    return 0;
}
