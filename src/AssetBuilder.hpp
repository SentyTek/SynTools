// ╒═══════════════════ AssetBuilder.hpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-09-13                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// │ Licensed under the MIT License       │
// ╰──────────────────────────────────────╯

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cstdint> // for scl

#include "../lib/miniscl.hpp"

namespace SynTools {

class AssetBuilder {
    scl::path projectPath;
    scl::path outputPath;
    scl::path shaderOutputPath;

  public:
    AssetBuilder(scl::path projectPath,
                 scl::path outputPath,
                 scl::path shaderOutputPath)
        : projectPath(std::move(projectPath)),
          outputPath(std::move(outputPath)),
          shaderOutputPath(std::move(shaderOutputPath)) {}
    ~AssetBuilder() = default;

    bool BuildAssets();

    bool BuildShaders();
    bool BuildMeshes();
    bool BuildGenerics();
    bool BuildGizmos();
};

class ShaderBuilder {
    struct ShaderPair {
        std::string vert;
        std::string frag;
        std::string vary;
        scl::path   path; // directory containing the shaders
    };

    std::vector<ShaderPair> shaderPairs;
    scl::path               sourcePath;
    scl::path               outputPath;
    scl::path               bundlePath;
    scl::path               includePath;
    bool                    singleBundle = false;
    std::string             singleBundleName;

  public:
    ShaderBuilder(scl::path   sourcePath,
                  scl::path   outputPath,
                  scl::path   bundlePath,
                  scl::path   includePath,
                  bool        singleBundle,
                  std::string singleBundleName)
        : sourcePath(std::move(sourcePath)), outputPath(std::move(outputPath)),
          bundlePath(std::move(bundlePath)),
          includePath(std::move(includePath)), singleBundle(singleBundle),
          singleBundleName(std::move(singleBundleName)) {}
    ~ShaderBuilder() = default;

    bool BuildShaders();

    void DiscoverShaderPairs();
    bool CompileShaderPairs();
    bool GenerateGroupMetadata();
    bool PackageGroups();
    void CleanUp();
};

} // namespace SynTools
