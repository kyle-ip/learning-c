# C Environment Setup

## For Windows (10)

-   Visual Studio (Community): [Download Visual Studio 2019 for Windows & Mac (microsoft.com)](https://visualstudio.microsoft.com/downloads/)
-   Msys2 (MinGW): [MSYS2](https://www.msys2.org/)

```shell
# mirror list:
# ./etc/pacman.d/{mirrorlist.msys,mirrorlist.mingw32,mirrorlist.mingw64}

pacman -Su
pacman -Sy base-devel

pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-binutils

pkg-config --list-all | grep xxx

# etc/nsswitch.conf > db_home: env windows xxx
```

-   WSL (version 2 is better): [Install WSL on Windows 10 | Microsoft Docs](https://docs.microsoft.com/en-us/windows/wsl/install-win10)

## For Linux (Ubuntu)

```shell
cp /etc/apt/sources.list /etc/apt/sources.list.bak 
cat > /etc/apt/sources.list << EOF
deb http://mirrors.aliyun.com/ubuntu/ trusty main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ trusty-security main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ trusty-updates main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ trusty-proposed main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ trusty-backports main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty-security main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty-updates main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty-proposed main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ trusty-backports main restricted universe multiverse
EOF

sudo apt -y update
sudo apt -y upgrade
sudo apt -y install gcc g++ gdb cmake
sudo apt autoremove
```

## Package Manager

Conan: [C/C++ Open Source Package Manager (conan.io)](https://conan.io/)

## IDE & Debugger

CLion: [CLion: A Cross-Platform IDE for C and C++ by JetBrains](https://www.jetbrains.com/clion/)

JetBrains Toolbox: [JetBrains Toolbox App: Manage Your Tools with Ease](https://www.jetbrains.com/toolbox-app/)

Configuration on CLion: 

-   Build, Execution, Deployment > Toolchains
-   Build, Execution, Deployment > CMake: Debug, Release ...

New Project:

-   New Project > C executable > Language standard: C11

### Debugger

LLDB (~/.lldbinit)

```shell
# Intel instruction 
settings set target.x86-disassembly-flavor intel
# AT&T instruction 
settings set target.x86-disassembly-flavor att
```

GDB (~/.gdbinit)

```shell
# Intel instruction 
set disassembly-flavor intel
# AT&T instruction 
set disassembly-flavor att
```

## GUI Development

Windows: 

```shell
# GTK3 and themes, glade
pacman -S mingw-w64-x86_64-gtk3
pacman -S mingw-w64-x86_64-adwaita-icon-theme
pacman -S mingw-w64-x86_64-glade
pacman -S mingw-w64-x86_64-zstd
```

Ubuntu: 

```shell
apt install -y \
gnome-tweak-tool \
gnome-shell-extension-autohidetopbar \
apt-transport-https \
ca-certificates \
curl \
gnupg-agent \
software-properties-common
```

## Release

to be continued.

