# PieceCake

A 3D Vulkan Renderer written in the most "loved" language, C++!

![Renderer Preview](assets/poc_screenshot.png "Renderer Preview")

## Install dependencies


### Arhc-Linux
```console
sudo pacman -S glfw glew glm vulkan-tools vulkan-validation-layers
```

### Ubuntu (TO-DO)
```console

```

## Compile and Run (TO-DO)
To compile and Run there is the need to install more dependencies.

### Arch-Linux
First, you have to specify where the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) compiler is.
```console
python run.py config --glslc_path '/usr/bin/glslc'
```

After you have just to run.
```console
python run.py config --run
```

### Windows
In Windows, I used [vcpkg](https://vcpkg.io/en/) and [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022).

#### Install dependencies (--RE-DO)
These instructions are outdated, you are probably going to install more than you need but they still work.

First, you need to download and install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows)
After you need to install the dependencies using your package manager.  
If you are using [vcpkg](https://vcpkg.io/en/), you can do it by this way.
```console
vcpkg install libigl[glfw]
vcpkg.exe install pkgconf:x64-windows
vcpkg.exe install glfw3:x64-windows glfw3:x86-windows glew:x64-windows glew:x86-windows
vcpkg install opengl glew glfw3 glm freetype eastl --triplet=x64-windows-static
vcpkg install vulkan --triplet=x64-windows-static
vcpkg.exe install glfw3 glfw3 glew glew --triplet=x64-windows-static
```

If you are using [vcpkg](https://vcpkg.io/en/), you also have to tell the [CMake](https://cmake.org/) where [vcpkg](https://vcpkg.io/en/) is.  
To do that you can use the [run.py](https://github.com/davidalmarinho/PieceOfCake/blob/main/run.py) script.  
Just run
```console
python run.py config --vcpkg_path 'C:/path_to_vcpkg/scripts/buildsystems/vcpkg.cmake'
```

To be able to compile the project, you have to specify the Vulkan Shader compiler path to the [run.py](https://github.com/davidalmarinho/PieceOfCake/blob/main/run.py) script inside the project directory.  
If you didn't change the path where you have installed the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home), you only have to execute this command.
```console
python run.py config --glslc_path 'C:/VulkanSDK/1.3.275.0/Bin/glslc'
```
If you have changed the path where you have installed [Vulkan SDK](https://vulkan.lunarg.com/sdk/home), you have to indicate the path where you have the 'glslc' compiler. It has to be inside the installation of your [Vulkan SDK](https://vulkan.lunarg.com/sdk/home).

Finally to run the project, just run:
```console
python run.py --run
```

## Include Configuration

If working on vscode, add this configuration to c_cpp_propertis.json file:
```json
"compileCommands": "${workspaceFolder}/compile_commands.json",
```

## Acknowledgments
[Vulkan Tutorial (Book)](https://vulkan-tutorial.com/Introduction)  
[Youtube Series additional resource that I used to follow the book.](https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR)  
[Multiple Object Rendering Article](https://mr-vasifabdullayev.medium.com/multiple-object-rendering-in-vulkan-3d07aa583cec)  
