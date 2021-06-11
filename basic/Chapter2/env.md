# 环境搭建

开发支持跨平台的 C 程序，在 Windows 下开发环境如下：

- Visual Studio (MSVC, C99)
- MSYS2 (MinGW, c11)
- WSL2 (gcc)
- CLion
- Conan

## Visual Studio

安装过程略，组件选 Desktop development with C++ 即可，目的是使用 MSVC 编译器。

## MSYS2

安装完成后，修改镜像源 `msys64/etc/pacman.d/{mirrorlist.msys,mirrorlist.mingw32,mirrorlist.mingw64}`：

```ini
Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/x86_64/
Server = http://mirrors.ustc.edu.cn/msys2/mingw/x86_64/
```

设置默认路径，修改 `msys64/etc/nsswitch.conf`：
```
db_home: env windows /C/Users/yipwi
# db_home: cygwin desc
```

安装依赖：
```shell
pacman -Su
pacman -Sy base-devel
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-binutils
pacman -S mingw-w64-x86_64-gtk3
pacman -S mingw-w64-x86_64-adwaita-icon-theme
pacman -S mingw-w64-x86_64-glade
pacman -S mingw-w64-x86_64-zstd

pkg-config --list-all | grep xxx
```

## WSL2

略，安装 gcc 和 g++：

```shell
apt install -y gcc g++ gdb cmake
```

## CLion

安装过程略，配置工具链和调试器：

- Toolchains：WSL (SSH), Visual Studio (amd64, x86), MinGW
- CMake: Debug (MinGW)

## Conan

使用 Python 的 pip 安装 Conan：

```shell
pip install conan
conan search libcurl --remote conan-center
```

在工程根目录下创建 `conanfile.txt`：

```ini
[requires]
libcurl/7.71.0

[generators]
cmake
```

安装依赖，在 `cmake-build-debug` 中执行：

```shell
conan install .. --profile=mingw64 --build=missing
```

在 `CMakeLists.txt` 下添加：

```cmake
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# ...
target_link_libraries(${name}
	# ...
	${CONAN_LIBS})
```

即可在代码中引入：

```c
#include <curl/curl.h>
```

