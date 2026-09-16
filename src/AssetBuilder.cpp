// ╒═══════════════════ AssetBuilder.cpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-09-13                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// │ Licensed under the MIT License       │
// ╰──────────────────────────────────────╯

#include "AssetBuilder.hpp"
#include "AssetPackager.hpp"
#include "ShaderCompiler.hpp"
#include "ShaderMeta.hpp"

#include <filesystem>
#include <iostream>
#include <unordered_map>

namespace SynTools {
namespace {

std::string StripSuffix(std::string value, const std::string& suffix) {
    if (value.size() >= suffix.size() &&
        value.compare(value.size() - suffix.size(), suffix.size(), suffix) ==
            0) {
        value.resize(value.size() - suffix.size());
    }
    return value;
}

scl::path ShaderCompilerPath() {
#ifdef _WIN32
    return scl::path::execdir() + "/shaderc.exe";
#else
    return scl::path::execdir() + "/shaderc";
#endif
}

scl::path GroupFor(const scl::path& sourcePath,
                   const scl::path& shaderPath,
                   bool             singleBundle) {
    if (singleBundle) {
        return scl::path("");
    }
    const scl::path shaderDirectory = shaderPath.parentpath();
    if (shaderDirectory == sourcePath) {
        return scl::path("assets_root");
    }
    scl::path   relativeDir  = shaderDirectory.relative(sourcePath);
    const char* relativeText = relativeDir.cstr();
    if (!relativeText || relativeText[0] == '\0' || relativeDir == ".") {
        return scl::path("assets_root");
    }
    return relativeDir.split().front();
}

bool CreateDirectories(const scl::path& directory) {
    std::error_code error;
    std::filesystem::create_directories(directory.cstr(), error);
    return !error;
}

} // namespace

bool AssetBuilder::BuildAssets() {
    return BuildShaders() && BuildMeshes() && BuildGenerics() && BuildGizmos();
}

bool AssetBuilder::BuildShaders() {
    const scl::path defaultSource = projectPath + "/engine/default/shaders";
    const scl::path gameSource    = projectPath + "/assets/shaders";

    ShaderBuilder defaultBuilder(defaultSource,
                                 shaderOutputPath + "/default",
                                 outputPath + "/shaders",
                                 projectPath +
                                     "/engine/third_party/bgfx.cmake/bgfx/src",
                                 true,
                                 "default_shaders");
    if (!defaultBuilder.BuildShaders()) {
        return false;
    }

    if (gameSource.isdirectory()) {
        ShaderBuilder gameBuilder(gameSource,
                                  shaderOutputPath,
                                  outputPath + "/shaders",
                                  projectPath +
                                      "/engine/third_party/bgfx.cmake/bgfx/src",
                                  false,
                                  "");
        if (!gameBuilder.BuildShaders()) {
            return false;
        }
    }
    return true;
}

bool AssetBuilder::BuildMeshes() {
    const scl::path source = projectPath + "/assets/meshes";
    if (!source.isdirectory()) {
        return true;
    }
    AssetPackager packager;
    return packager.PackTree(source.cstr(),
                             (outputPath + "/meshes").cstr(),
                             { "--root-bundle-name=meshes" });
}

bool AssetBuilder::BuildGenerics() {
    const scl::path assets = projectPath + "/assets";
    if (!assets.isdirectory()) {
        return true;
    }

    AssetPackager packager;
    for (const auto& entry :
         scl::path::glob(assets + "/*", scl::GlobMode::DIRS)) {
        const std::string name = entry.filename().cstr();
        if (name == "meshes" || name == "shaders") {
            continue;
        }
        if (!packager.PackTree(entry.cstr(),
                               (outputPath + "/" + name).cstr(),
                               { "--root-bundle-name=" + name })) {
            return false;
        }
    }
    return true;
}

bool AssetBuilder::BuildGizmos() {
    const scl::path source = projectPath + "/engine/default/gizmos";
    if (!source.isdirectory()) {
        return true;
    }

    const scl::path bundle = outputPath + "/gizmos/default_gizmos.spk";
    AssetPackager   packager;
    return packager.CreateFileBundle(
        { "." },
        bundle.cstr(),
        "",
        { "--src-dir=" + std::string(source.cstr()) });
}

bool ShaderBuilder::BuildShaders() {
    if (!sourcePath.isdirectory()) {
        return true;
    }
    if (!CreateDirectories(outputPath)) {
        return false;
    }

    CleanUp();
    DiscoverShaderPairs();
    if (!CompileShaderPairs() || !GenerateGroupMetadata()) {
        return false;
    }
    return PackageGroups();
}

void ShaderBuilder::DiscoverShaderPairs() {
    std::vector<scl::path> shaderDirectories{ sourcePath };
    const auto             nestedDirectories =
        scl::path::glob(sourcePath + "/**", scl::GlobMode::DIRS);
    shaderDirectories.insert(shaderDirectories.end(),
                             nestedDirectories.begin(),
                             nestedDirectories.end());

    for (const auto& directory : shaderDirectories) {
        for (const auto& fragment : scl::path::glob(directory + "/*.frag.bgsl",
                                                    scl::GlobMode::FILES)) {
            const std::string filename  = fragment.filename().cstr();
            const std::string base      = StripSuffix(filename, ".frag.bgsl");
            const scl::path   directory = fragment.parentpath();
            const scl::path   vertex    = directory + "/" + base + ".vert.bgsl";
            const scl::path   varying =
                directory + "/varying/" + base + ".vary.bgsl";

            if (!vertex.isfile()) {
                std::cerr << "Warning: no matching vertex shader for "
                          << fragment.cstr() << std::endl;
                continue;
            }
            if (!varying.isfile()) {
                std::cerr << "Warning: no matching varying file for "
                          << fragment.cstr() << std::endl;
                continue;
            }

            ShaderPair pair;
            pair.vert = vertex.relative(sourcePath).cstr();
            pair.frag = fragment.relative(sourcePath).cstr();
            pair.vary = varying.relative(sourcePath + "/varying").cstr();
            pair.path = directory;
            shaderPairs.push_back(std::move(pair));
        }
    }
}

bool ShaderBuilder::CompileShaderPairs() {
    const scl::path compiler = ShaderCompilerPath();

    for (const ShaderPair& pair : shaderPairs) {
        const scl::path   fragment(pair.frag);
        const scl::path   vertex(pair.vert);
        const std::string fragmentKey =
            StripSuffix(fragment.cstr(), ".frag.bgsl");
        const std::string vertexKey  = StripSuffix(vertex.cstr(), ".vert.bgsl");
        const scl::path   shaderPath = sourcePath + "/" + pair.frag;
        const scl::path group = GroupFor(sourcePath, shaderPath, singleBundle);
        std::string     relativeBase = StripSuffix(pair.frag, ".frag.bgsl");
        if (!singleBundle && group != "assets_root") {
            const std::string prefix = std::string(group.cstr()) + "/";
            if (relativeBase.rfind(prefix, 0) == 0) {
                relativeBase.erase(0, prefix.size());
            }
        }
        const scl::path outputBase = group == ""
                                         ? outputPath + "/" + relativeBase
                                         : outputPath + "/" + group + "/" + relativeBase;

        if (!CreateDirectories(outputBase.parentpath())) {
            return false;
        }
        std::vector<std::string> options{
            "--compiler=" + std::string(compiler.cstr()),
            "--src-dir=" + std::string(sourcePath.cstr()),
            "--src-ext=.bgsl",
            "--include=" + std::string(includePath.cstr()),
            "--no-default-include",
            "--varying=" + StripSuffix(pair.vary, ".bgsl")
        };

        if (!ShaderCompiler::CompileShader(
                fragmentKey, vertexKey, outputBase.cstr(), options)) {
            return false;
        }
    }
    return true;
}

bool ShaderBuilder::GenerateGroupMetadata() {
    if (shaderPairs.empty()) {
        return true;
    }

    std::unordered_map<std::string, scl::path> groups;
    for (const ShaderPair& pair : shaderPairs) {
        const scl::path shaderPath = sourcePath + "/" + pair.frag;
        const scl::path group = GroupFor(sourcePath, shaderPath, singleBundle);
        groups.emplace(group.cstr(), outputPath + "/" + group);
    }

    for (const auto& [group, directory] : groups) {
        if (!CreateDirectories(directory)) {
            return false;
        }
        const scl::path source = singleBundle || group.empty() ||
                         group == "assets_root"
                                     ? sourcePath
                                     : sourcePath + "/" + group;
        if (!ShaderMeta::GenerateShaderMeta(
                source.cstr(),
                { "--output=" + std::string(directory.cstr()) })) {
            return false;
        }
    }
    return true;
}

bool ShaderBuilder::PackageGroups() {
    std::unordered_map<std::string, std::vector<std::string>> files;
    for (const ShaderPair& pair : shaderPairs) {
        const scl::path shaderPath = sourcePath + "/" + pair.frag;
        const scl::path group = GroupFor(sourcePath, shaderPath, singleBundle);
        std::string     base  = StripSuffix(pair.frag, ".frag.bgsl");
        if (!singleBundle && group != "assets_root") {
            const std::string prefix = std::string(group.cstr()) + "/";
            if (base.rfind(prefix, 0) == 0) {
                base.erase(0, prefix.size());
            }
        }
        files[group.cstr()].push_back(base + ".vert.bin");
        files[group.cstr()].push_back(base + ".frag.bin");
        files[group.cstr()].push_back("meta.xml");
    }

    AssetPackager packager;
    if (!CreateDirectories(bundlePath)) {
        return false;
    }
    for (const auto& [group, entries] : files) {
        const std::string name   = singleBundle ? singleBundleName : group;
        const scl::path   source = outputPath + "/" + group;
        if (!packager.CreateFileBundle(
                entries,
                (bundlePath + "/" + name + ".spk").cstr(),
                "",
                { "--src-dir=" + std::string(source.cstr()),
                  "--skip-discovery" })) {
            return false;
        }
    }
    return true;
}

void ShaderBuilder::CleanUp() { shaderPairs.clear(); }

} // namespace SynTools
