# SynTools
CLI tools used to help make game development with Syngine and Syngine Studio easier. Includes Shader compiler, Asset packager/unpackager, and Scene packager.

# Usage:
## Shader compilation
`syntools shader <fragment_name> <vertex_name> <output_path> [options]`

### Options:

`--compiler=<path>` Path to bgfx shaderc exec, relative to program. (default: ./shaderc). You shouldn't have to change this, as by default, shaderc and syntools are in the same directory.

`--varying=<path>` Path to varying def file. By default, it looks in `fragment_path/varying/fragment_name.vary.[ext]`.

`--src-ext=<path>` The extension of the shader source files. Default is `[frag/vert/vary].sc`

`--out-ext=<path>` The extension of the output binary file. Default `[vert/frag].bin`

`--src-dir=<path>` Source directory for the shaders, relative to program. Regular syntax, `..` and such supported. Default is `gameProject/assets/shaders`

`--include=<path>` Additional includes for shaders. Relative to program, multiple allowed.

### Examples:
`syntools shader space space space_output`
- Will compile shader with fragment & vertex named `space.frag.sc`/`space.vert.sc` into `space_output.bin`
- Assumes shader sources are in same dir as `syntools`, will output to same dir as well.

`syntools shader terrain s s --src-dir=../../assets/shaders/world --src-ext=.shadercode`
- Will compile shader with fragment & vertex named `terrain.frag.shadercode`/`terrain.vert.shadercode` into `terrain.[frag/vert].bin`
- Shader source path is `../../assets/shaders/world`
- Shader extension is `.shadercode`
- Note the replacement of the vertex and output with `s` (means "same"). If either of these are `s`, it will assume the vertex and/or output names are the same as the fragment's name.

## File bundling
`syntools pack <output_file> <file1> <file2> ... [options]`
- Note that`file` can be a relative path to either a literal file, or to a directory, where every file in said dir (non-recursive) will be packaged.

### Options:
`--src-dir=<path>` Path where every file/directory will then be searched from. Very useful for the relative asset paths inside the game. e.g. `--src-dir=./Bakerman.app/Contents/Resources` with `file1` being `shaders` will look for files in src-dir/shaders. This is useful for multi-platform development, as keeping relative paths in the package is essential.

### Example:
`syntools pack meshes.spk meshes`
- Will package all files in `./meshes` into `meshes.spk`

## Asset packaging

## Scene packaging

## Usage in CMake scripts as a post-build step

# Technologies Used
[SCL](https://github.com/MerianBerry/SCL)