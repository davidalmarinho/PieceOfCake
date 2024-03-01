# PieceCake

## Requirement packages

```console
sudo apt-get install libglew-dev  

Install vulkan-sdk # <--- Este não é instalação!!! TODO TODO

Ubuntu:
sudo apt-get install pkg-config
sudo apt-get install libglfw3-dev
sudo apt-get install libglew-dev

Arch:
pacman -Syu cmake
pamac install glfw-x11
sudo pacman -S vulkan-tools
sudo pacman -S vulkan-validation-layers
sudo pacman -S valgrind

In arch linux you have to install "glm-git" instead "glm" from Arch Linux User Repository (AUR)
git clone https://aur.archlinux.org/glm-git.git
cd glm-git
makepkg --install

sudo pacman -S sdl2-image -> Acho que não é necessário
sudo pacman -S sdl2 Acho -> que não é necessário
```

```console
    WINDOWS:
    vcpkg install libigl[glfw]
    vcpkg.exe install pkgconf:x64-windows
vcpkg.exe install glfw3:x64-windows glfw3:x86-windows glew:x64-windows glew:x86-windows
vcpkg install opengl glew glfw3 glm freetype eastl --triplet=x64-windows-static
vcpkg install vulkan --triplet=x64-windows-static
vcpkg.exe install glfw3 glfw3 glew glew --triplet=x64-windows-static

---
1º Install VulkanSDK

2º
Install vcpkg and install this packages with it:
vcpkg.exe integrate install

REALLY NECCESSARY:
	vcpkg.exe install pkgconf:x64-windows
	vcpkg.exe install glfw3:x64-windows-static

vcpkg.exe install glfw3 --triplet=x64-windows-static
vcpkg.exe install glew:x64-windows
vcpkg.exe install glm
vcpkg.exe install glm:x86-windows
~vcpkg.exe install glew
~vcpkg.exe install glew --triplet=x64-windows-static
X vcpkg install vulkan --triplet=x64-windows-static

running: cmake .. -DCMAKE_TOOLCHAIN_FILE=D:/Softwares/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
compiling: msbuild SOLUTION.sln
```

## Include Configuration

If working on vscode, add this configuration to c_cpp_propertis.json file:

```json
"compileCommands": "${workspaceFolder}/compile_commands.json",
```
