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
```

```console
    WINDOWS:
    vcpkg install libigl[glfw]
```

## Include Configuration

If working on vscode, add this configuration to c_cpp_propertis.json file:

```json
"compileCommands": "${workspaceFolder}/compile_commands.json",
```
