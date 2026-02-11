// ╒══════════════════ AssetPackager.cpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-01-30                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// │ Placeholder License                  │
// ╰──────────────────────────────────────╯

#include "AssetPackager.hpp"
#include <iostream>
#include <filesystem>
#include <system_error>
#define SCL_IMPL
#include "../lib/miniscl.hpp"

namespace SynTools {

AssetPackager::AssetPackager() { scl::init(); }
AssetPackager::~AssetPackager() { scl::terminate(); }


bool AssetPackager::CreateFileBundle(const std::vector<std::string>& files,
                                     const std::string&              outputPath,
                                     const std::string&              execPath,
                                     const std::vector<std::string>& options) {
    namespace fs = std::filesystem;
    // Validate files
    std::vector<scl::path> entries; // Relative paths
    std::vector<scl::path> diskPaths; // Absolute disk paths

    // Set output path to absolute before changing working dir, so it can be relative to execPath if needed
    scl::path output(fs::weakly_canonical(fs::path(outputPath)).c_str());

    fs::path baseDir = fs::current_path();
    for (const std::string& option : options) {
        if (option.rfind("--src-dir=", 0) == 0) {
            std::string dirPath = option.substr(10);
            if (!dirPath.empty()) {
                try {
                    fs::current_path(dirPath); // Change current path to the source directory
                    // baseDir = fs::weakly_canonical(fs::path(dirPath));
                    baseDir = fs::current_path();
                    std::cout << "Set source directory to: " << baseDir.string() << std::endl;
                } catch (const fs::filesystem_error& e) {
                    std::cout << "Error setting source directory: " << e.what() << std::endl;
                    return false;
                }
            }
        }
    }
    
    for (const std::string& file : files) {
        std::cout << "Processing file: " << file << std::endl;
        scl::path finalPath(file.c_str());

        // Supplying a path like `shaders/` or `./shaders` won't work with SCL,
        // so we convert it to absolute ourselves and then check if a file or
        // dir. Ngl this is a bit hacky but whatever.
        fs::path absPath;
        try {
            fs::path dirPath = fs::path(baseDir.string() + "/" + file);
            absPath = fs::weakly_canonical(dirPath);
            finalPath        = scl::path(absPath.string().c_str());
            std::cout << "\t(resolved to absolute path: " << absPath.string()
                      << ")" << std::endl;
        } catch (fs::filesystem_error& e) {
            std::cout << "Error processing path: " << e.what() << std::endl;
            return false;
        }
        
        if (!finalPath.exists()) { // .exists() works for both files and dirs
            std::cout << "File not found: " << finalPath.cstr() << std::endl;
            return false;
        }

        // Helper to separate source disk paths from relative paths
        auto registerFile = [&](const fs::path& fullpath) {
            diskPaths.push_back(scl::path(fullpath.string().c_str()));

            // Get relative path for the archive entry
            std::error_code ec;
            fs::path rel = fs::relative(fullpath, baseDir, ec);
            if (ec) { // Fallback to filename only
                rel = fullpath.filename();
            }
            entries.push_back(scl::path(rel.string().c_str()));
        };
        
        // Since 'file' may be a dir, we need to check and add all files
        // within it (explicitly NOT recursive)
        if (finalPath.isdirectory()) {
            auto dirFiles = scl::path::glob(finalPath + "/*");
            for (const auto& f : dirFiles) {
                registerFile(fs::path(f.cstr()));
            }
            continue;
        }
        registerFile(absPath);
    }
    
    std::cout << "Packaging " << entries.size() << " files" << std::endl;
    for (const auto& f : entries) {
        std::cout << "Adding to package: " << f.cstr() << std::endl;
        std::cout << "\t(abs: " << diskPaths[&f - &entries[0]].cstr() << ")" << std::endl;
    }

    // If package already exists, delete it before creating a new one
    if (output.exists()) {
        std::cout << "Output file already exists, deleting: " << output.cstr() << std::endl;
        std::error_code ec;
        fs::remove(output.cstr(), ec);
        if (ec) {
            std::cout << "Error deleting existing output file: " << ec.message() << std::endl;
            return false;
        }
    }
    
    // Insert files into package
    double clock = scl::clock();
    scl::pack::Packager pack;
    pack.open(output);
    auto result = pack.openFiles(entries);

    for (auto i : result) {
        i->submit();
    }
    
    pack.write();
    std::cout << "Packaging completed in " << ((scl::clock() - clock) * 1000)
              << " milliseconds." << std::endl;

    pack.open(output);

    // Validate package
    auto index = pack.index();

    if (index.size() != entries.size()) {
        std::cout << "Package validation failed." << std::endl;
        pack.close();
        return false;
    }
    std::cout << "Package validation succeeded." << std::endl;
    pack.close();
    
    return true;
}

void AssetPackager::PrintHelp() {
    std::cout << "Asset Packager Help:" << std::endl;
    std::cout << "Usage: packager <output_path> [options] <file1> <file2> ..." << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help        Show this help message" << std::endl;
    std::cout << "  --src-dir=        Specify the source directory for assets" << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Packages multiple asset files into a single bundle." << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout
        << "\t syntools pack assets.bundle texture.png model.obj"
        << std::endl;
}

void AssetPackager::ValidatePackage(const std::string& packagePath) {
    scl::pack::Packager pack;
    if (!pack.open(packagePath.c_str())) {
        std::cout << "Failed to open package: " << packagePath << std::endl;
        return;
    }

    auto index = pack.index();
    int  totalSize = 0;
    int  totalOriginal = 0;
    std::cout << "Package contains " << index.size() << " files:" << std::endl;
    for (const auto& entry : index) {
        totalSize += entry->compressed();
        totalOriginal += entry->original();
        std::cout << " - " << entry->filepath().cstr() << " (" << entry->compressed()
                  << " / " << entry->original() << " bytes) "
                  << (int)((double)entry->compressed() / (double)entry->original() * 100.0)
                  << "% compressed)" << std::endl;
    }

    std::cout << "Total size: " << totalSize << " / " << totalOriginal
              << " bytes (" << (int)((double)totalSize / (double)totalOriginal * 100.0)
              << "% compressed)" << std::endl;

    std::cout << "Package validated successfully." << std::endl;
    pack.close();
    return;
}
}; // namespace SynTools
