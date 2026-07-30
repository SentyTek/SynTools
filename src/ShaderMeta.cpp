// ╒═════════════════════ ShaderMeta.cpp ═╕
// │ Syngine Tools                        │
// │ Created 2026-07-28                   │
// ├──────────────────────────────────────┤
// │ Copyright (c) SentyTek 2025-2026     │
// │ Licensed under the MIT License       │
// ╰──────────────────────────────────────╯

#include "ShaderMeta.hpp"

#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_set>

#include "../lib/miniscl.hpp"

#define SYNINT_SHADER_METADATA_VERSION "1.0"

namespace SynTools {

// If a file has an #include directive, this function will recursively read the
// included file and replace the directive with the contents of the included
// file.
void ShaderMeta::_preprocessFile(const std::filesystem::path& filePath,
                                 std::string&                 outContent,
                                 bool                         verbose) {
    static std::unordered_set<std::string> processingStack;

    std::filesystem::path resolvedPath = std::filesystem::absolute(filePath);
    std::string           resolvedKey  = resolvedPath.string();

    if (processingStack.contains(resolvedKey)) {
        return;
    }

    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        std::cerr << "Warning: Unable to open shader file: " << resolvedPath
                  << std::endl;
        return;
    }

    processingStack.insert(resolvedKey);

    std::string line;
    std::regex  includeRegex(R"(^\s*#include\s+(<[^>]+>|"([^"]+)\").*)");
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_match(line, match, includeRegex)) {
            std::string includeToken = match[1].str();
            std::string includePathStr;
            if (!includeToken.empty() && includeToken.front() == '<') {
                includePathStr =
                    includeToken.substr(1, includeToken.size() - 2);
            } else if (!includeToken.empty() && includeToken.front() == '"') {
                includePathStr =
                    includeToken.substr(1, includeToken.size() - 2);
            }

            if (!includePathStr.empty()) {
                if (includePathStr == "bgfx_shader.sh") {
                    outContent += line;
                    outContent += '\n';
                    continue;
                }

                std::filesystem::path includePath(includePathStr);
                std::filesystem::path candidatePath =
                    resolvedPath.parent_path() / includePath;
                std::filesystem::path fallbackPath = includePath;

                if (!std::filesystem::exists(candidatePath)) {
                    candidatePath = fallbackPath;
                }

                if (std::filesystem::exists(candidatePath)) {
                    std::string includedContent;
                    _preprocessFile(candidatePath, includedContent, verbose);
                    if (verbose) {
                        std::cout << "Included '" << includePathStr
                                  << "' into '"
                                  << resolvedPath.filename().string() << "'"
                                  << std::endl;
                    }
                    outContent += includedContent;
                    if (!includedContent.empty() &&
                        includedContent.back() != '\n') {
                        outContent += '\n';
                    }
                } else {
                    outContent += line;
                    outContent += '\n';
                }
            } else {
                outContent += line;
                outContent += '\n';
            }
        } else {
            outContent += line;
            outContent += '\n';
        }
    }

    processingStack.erase(resolvedKey);
}

void ShaderMeta::_parseFile(const std::string& content, _Shader& shader) {
    // Regex to match: // @<Tag> [<key>="<value>" ...]
    std::regex annotationRegex(R"(//\s*@([A-Za-z0-9_]+)(?:\s+(.*))?)");
    // Regex to extract key="value" pairs
    std::regex         kvRegex("([A-Za-z0-9_]+)=\"([^\"]*)\"");
    std::string        line;
    std::string        currentTag = "";
    std::istringstream stream(content);

    while (std::getline(stream, line)) {
        std::smatch  match;
        _UniformMeta uniformMeta;

        // Did we hit an annotation line?
        if (std::regex_search(line, match, annotationRegex)) {
            currentTag = match[1].str(); // e.g. "EngineUniform"
            std::string kvString =
                match[2].str(); // e.g. 'name="u_time" type="float"
                                // frequency="frame"'
            std::string nextLine;
            std::getline(stream,
                         nextLine); // Read the next line to get the uniform
                                    // declaration

            // Extract key-value pairs
            auto kvBegin =
                std::sregex_iterator(kvString.begin(), kvString.end(), kvRegex);
            auto kvEnd = std::sregex_iterator();

            for (auto kvIt = kvBegin; kvIt != kvEnd; ++kvIt) {
                std::string key   = (*kvIt)[1].str();
                std::string value = (*kvIt)[2].str();

                if (key == "freq") {
                    uniformMeta.frequency = value;
                } else if (key == "src") {
                    uniformMeta.src = value;
                } else if (key == "default") {
                    uniformMeta.defaultValue = value;
                } else if (key == "num") {
                    uniformMeta.num = value;
                } else if (key == "name") {
                    uniformMeta.name = value;
                } else if (key == "stage") {
                    uniformMeta.texStage = value;
                } else {
                    std::cerr
                        << "Warning: Unrecognized key in annotation: " << key
                        << std::endl;
                }
            }

            // get the type and name on the next line
            std::string uniformKeyword, type, name;
            if (!currentTag.empty() &&
                nextLine.find("uniform") != std::string::npos) {
                std::istringstream iss(nextLine);
                iss >> uniformKeyword >> type >> name;
                // Remove the trailing semicolon from the name
                if (!name.empty() && name.back() == ';') {
                    name.pop_back();
                }

                if (uniformMeta.name.empty()) {
                    uniformMeta.name = name;
                }

                // Strip array suffixes from uniform names such as
                // u_csmLightViewProj[4]
                auto bracketPos = uniformMeta.name.find('[');
                if (bracketPos != std::string::npos) {
                    uniformMeta.name.erase(bracketPos);
                }

                uniformMeta.type = type;
            }
        } else {
            continue;
        }

        // Discard if we've already seen this uniform name in this shader
        bool duplicate = false;
        for (const auto& existingUniform : shader.EngineUniforms) {
            if (existingUniform.name == uniformMeta.name) {
                duplicate = true;
                break;
            }
        }
        for (const auto& existingUniform : shader.EngineSamplers) {
            if (existingUniform.name == uniformMeta.name) {
                duplicate = true;
                break;
            }
        }
        for (const auto& existingUniform : shader.MaterialParams) {
            if (existingUniform.name == uniformMeta.name) {
                duplicate = true;
                break;
            }
        }
        for (const auto& existingUniform : shader.Samplers) {
            if (existingUniform.name == uniformMeta.name) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            continue;
        }

        if (currentTag == "EngineUniform") {
            shader.EngineUniforms.push_back(uniformMeta);
        } else if (currentTag == "EngineSampler") {
            shader.EngineSamplers.push_back(uniformMeta);
        } else if (currentTag == "MaterialParam" ||
                   currentTag == "MaterialParameter") {
            shader.MaterialParams.push_back(uniformMeta);
        } else if (currentTag == "Sampler") {
            shader.Samplers.push_back(uniformMeta);
        } else {
            std::cerr << "Warning: Unrecognized annotation tag: " << currentTag
                      << std::endl;
        }
    }
}

void ShaderMeta::PrintHelp() {
    std::string url =
        "https://github.com/SentyTek/Syngine/blob/main/docs/index.md";
    std::string name = "Syngine Shader Documentation";
    std::cout
        << "Usage: synginetools shadermeta <path_to_shader_dir> [options]\n";
    std::cout << "Generates metadata for the specified shader directory.\n";
    std::cout
        << "Please refer to the " << "\033]8;;" << url << "\033\\" << name
        << "\033]8;;\033\\"
        << " for details on writing shaders to generate correct metadata.\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --help, -h             Show this help message\n";
    std::cout << "  --verbose              Enable verbose output\n";
    std::cout << "  --output= | -o=<file>  Specify output file path (Will "
                 "always be path/meta.xml)\n";
}

// syntools.exe sm ../../../engine/default/shaders
// ./syntools sm ../../../engine/default/shaders

bool ShaderMeta::GenerateShaderMeta(const std::string& shaderDirPath,
                                    const std::vector<std::string>& options) {

    bool      verbose = false;
    scl::path outputPath(shaderDirPath);

    for (std::string op : options) {
        if (op == "--help" || op == "-h") {
            PrintHelp();
            return true;
        } else if (op == "--verbose") {
            verbose = true;
        } else if (op.rfind("--output=", 0) == 0) {
            outputPath = scl::path(op.substr(9));
        } else if (op.rfind("-o=", 0) == 0) {
            outputPath = scl::path(op.substr(3));
        }
    }

    // Check if the shader files exist
    std::vector<scl::path> files =
        scl::path::glob(shaderDirPath + "/*", scl::GlobMode::FILES);

    std::vector<std::string> shaderFiles;
    for (const auto& file : files) {
        if (file.extension() == ".vert.bgsl") {
            shaderFiles.push_back(file.cstr());
        }
    }

    std::string          line;
    std::string          currentTag = "";
    std::vector<_Shader> shaders;

    for (const auto& vsPath : shaderFiles) {
        if (verbose) {
            std::cout << "Generating shader metadata for: "
                      << std::filesystem::path(vsPath).stem() << std::endl;
        }

        _Shader shader;
        shader.name =
            scl::path(vsPath).stem().cstr(); // Use the filename without
                                             // extension as shader name

        std::string preprocessedVs;
        std::string preprocessedFs;

        // unfortunately its not super easy to replace the file extension
        // from
        // `.vert.bgsl` to `.frag.bgsl`, most functions only treat the
        // `.bgsl` as the extensoin, so we have to do a bit of a hacky
        // replace here
        std::string fragPath =
            scl::string(vsPath).replace(".vert.bgsl", ".frag.bgsl").cstr();

        _preprocessFile(std::filesystem::path(vsPath), preprocessedVs, verbose);
        _preprocessFile(
            std::filesystem::path(fragPath), preprocessedFs, verbose);

        _parseFile(preprocessedVs, shader);
        _parseFile(preprocessedFs, shader);

        int totalUniforms = shader.EngineUniforms.size() +
                            shader.MaterialParams.size() +
                            shader.Samplers.size();
        if (verbose) {
            std::cout << "Parsed " << totalUniforms << " uniforms from shader."
                      << std::endl;
        }
        shaders.push_back(shader);
    }

    int totalUniforms = 0;
    for (const auto& shader : shaders) {
        totalUniforms += shader.EngineUniforms.size() +
                         shader.MaterialParams.size() + shader.Samplers.size();
    }
    std::cout << "Generated metadata for " << shaders.size() << " shaders, "
              << totalUniforms << " uniforms total." << std::endl;

    // create XML document
    scl::xml::XmlDocument doc;
    auto* v = doc.new_attr("Version", SYNINT_SHADER_METADATA_VERSION);
    doc.set_tag("SyngineShaderMetadata");
    doc.add_attr(v);

    for (auto& shader : shaders) {
        auto* shaderElem = doc.new_elem("Shader");
        auto* nameAttr   = doc.new_attr("name", shader.name);
        shaderElem->add_attr(nameAttr);

        auto* uniformsElem = doc.new_elem("Uniforms");
        shaderElem->add_child(uniformsElem);

        for (auto& uniform : shader.EngineUniforms) {
            auto* uniformElem = doc.new_elem("Uniform");
            uniformsElem->add_child(uniformElem);

            // Validate required attributes
            if (uniform.name.empty() || uniform.type.empty() ||
                uniform.frequency.empty() || uniform.src.empty()) {
                std::cerr << "Error: Missing required attributes for "
                             "EngineUniform '"
                          << uniform.name << "' in shader '" << shader.name
                          << "'" << std::endl;
                continue;
            }

            auto* nameAttr = doc.new_attr("name", uniform.name);
            auto* typeAttr = doc.new_attr("type", uniform.type);
            auto* freqAttr = doc.new_attr("freq", uniform.frequency);
            auto* srcAttr  = doc.new_attr("src", uniform.src);

            uniformElem->add_attr(nameAttr);
            uniformElem->add_attr(typeAttr);
            uniformElem->add_attr(freqAttr);
            uniformElem->add_attr(srcAttr);

            if (!uniform.num.empty()) {
                auto* numAttr = doc.new_attr("num", uniform.num);
                uniformElem->add_attr(numAttr);
            }
        }

        auto* materialParamsElem = doc.new_elem("MaterialParams");
        shaderElem->add_child(materialParamsElem);
        for (auto& param : shader.MaterialParams) {
            auto* paramElem = doc.new_elem("Param");
            materialParamsElem->add_child(paramElem);

            // Validate required attributes
            if (param.name.empty() || param.type.empty()) {
                std::cerr << "Error: Missing required attributes for "
                             "MaterialParam '"
                          << param.name << "' in shader '" << shader.name << "'"
                          << std::endl;
                continue;
            }

            if (param.num.empty())
                param.num = "1"; // Default to 1 if not specified

            auto* nameAttr = doc.new_attr("name", param.name);
            auto* typeAttr = doc.new_attr("type", param.type);
            auto* numAttr  = doc.new_attr("num", param.num);

            paramElem->add_attr(nameAttr);
            paramElem->add_attr(typeAttr);
            paramElem->add_attr(numAttr);
        }

        auto* samplersElem = doc.new_elem("Samplers");
        shaderElem->add_child(samplersElem);
        for (auto& sampler : shader.Samplers) {
            auto* samplerElem = doc.new_elem("Sampler");
            samplersElem->add_child(samplerElem);

            // Validate required attributes
            if (sampler.name.empty()) {
                std::cerr << "Error: Missing name attribute for "
                             "Sampler in shader '"
                          << shader.name << "'" << std::endl;
                continue;
            }
            if (sampler.texStage.empty()) {
                std::cerr << "Error: Missing stage attribute for "
                             "Sampler '"
                          << sampler.name << "' in shader '" << shader.name
                          << "'" << std::endl;
                continue;
            }

            auto* nameAttr = doc.new_attr("name", sampler.name);
            samplerElem->add_attr(nameAttr);
            auto* stageAttr = doc.new_attr("stage", sampler.texStage);
            samplerElem->add_attr(stageAttr);
        }

        auto* engineSamplersElem = doc.new_elem("EngineSamplers");
        shaderElem->add_child(engineSamplersElem);
        for (auto& sampler : shader.EngineSamplers) {
            auto* samplerElem = doc.new_elem("Sampler");
            engineSamplersElem->add_child(samplerElem);

            // Validate required attributes
            if (sampler.name.empty()) {
                std::cerr << "Error: Missing name attribute for "
                             "EngineSampler in shader '"
                          << shader.name << "'" << std::endl;
                continue;
            }
            if (sampler.texStage.empty()) {
                std::cerr << "Error: Missing stage attribute for "
                             "EngineSampler '"
                          << sampler.name << "' in shader '" << shader.name
                          << "'" << std::endl;
                continue;
            }
            if (sampler.src.empty()) {
                std::cerr << "Error: Missing src attribute for "
                             "EngineSampler '"
                          << sampler.name << "' in shader '" << shader.name
                          << "'" << std::endl;
                continue;
            }

            auto* nameAttr = doc.new_attr("name", sampler.name);
            samplerElem->add_attr(nameAttr);
            auto* stageAttr = doc.new_attr("stage", sampler.texStage);
            samplerElem->add_attr(stageAttr);
            auto* srcAttr = doc.new_attr("src", sampler.src);
            samplerElem->add_attr(srcAttr);
        }

        doc.add_child(shaderElem);
    }

    scl::stream file;
    file.open(outputPath / "meta.xml", scl::OpenMode::RWTRUNC, true);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open output file for writing: "
                  << (outputPath / "meta.xml").cstr() << std::endl;
        return false;
    }
    auto res = doc.print(file);
    if (res.code != scl::xml::OK) {
        std::cerr << "Error: Failed to write XML document to file: "
                  << (outputPath / "meta.xml").cstr() << std::endl;
        return false;
    }
    file.flush();
    file.close();

    return true;
}

} // namespace SynTools
