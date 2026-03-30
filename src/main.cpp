// ╒═══════════════════════════ main.cpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-01-30                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// | Licensed under the MIT License       |
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
    std::cout << "  pack         Compress & package game files\n";
    std::cout << "  validate     Validate a packaged asset bundle\n";
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
    if (command == "shader" || command == "shaders") {
        if (argc < 5 || std::string(argv[2]) == "--help" || std::string(argv[2]) == "-h") {
            SynTools::ShaderCompiler::PrintHelp();
            return 0;
        }
        std::string fragmentPath = argv[2];
        std::string vertexPath   = argv[3];
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
    } else if (command == "pack" || command == "package" || command == "packager") {
        if (argc < 3 || std::string(argv[2]) == "--help" || std::string(argv[2]) == "-h") {
            SynTools::AssetPackager packager;
            packager.PrintHelp();
            return 0;
        }
        std::string outputPath = argv[2];
        std::vector<std::string> files;
        std::vector<std::string> options;
        for (int i = 3; i < argc; ++i) {
            // If option
            if (std::string(argv[i]).rfind("-", 0) == 0) {
                options.push_back(argv[i]);
                continue;
            }
            files.push_back(argv[i]);
        }
        SynTools::AssetPackager packager;
        bool success = packager.CreateFileBundle(files, outputPath, std::string(argv[0]), options);
        if (!success) {
            std::cerr << "Asset packaging failed." << std::endl;
            return 1;
        }
        std::cout << "Asset(s) packaged successfully." << std::endl;
    } else if (command == "validate") {
        if (argc < 3 || std::string(argv[2]) == "--help" || std::string(argv[2]) == "-h") {
            std::cout << "Usage: syngine_tools validate <package_path>\n";
            return 0;
        }
        std::string packagePath = argv[2];
        SynTools::AssetPackager packager;
        packager.ValidatePackage(packagePath);
    } else {
        printHelp();
        return 1;
    }
    
    return 0;
}
