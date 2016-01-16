LIBCLANG 3.7.1

BUILD ENVIRONMENT
=================
> Visual Studio 2015
> CMake (https://cmake.org/download/)
> Python (https://www.python.org/downloads/)

SOURCE CODE
===========
> Download: http://llvm.org/releases/3.7.1/llvm-3.7.1.src.tar.xz
> Download: http://llvm.org/releases/3.7.1/cfe-3.7.1.src.tar.xz
> Extract llvm-3.7.1.src.tar.xz {somewhere}\llvm-3.7.1.src
> Extract cfe-3.7.1.src.tar.xz into {somewhere}\llvm-3.7.1.src\tools\cfe-3.7.1.src
> Rename cfe-3.7.1.src directory to clang

HEADERS
=======
> {somewhere}\llvm-3.7.1.src\tools\clang\include\clang-c

BUILD (x86)
===========
> Open Developer Command Prompt for Visual Studio
> CD {somewhere}
> MD llvm-3.7.1-build
> CD llvm-3.7.1-build
cmake -G "Visual Studio 14" ..\llvm-3.7.1.src\ -DLLVM_TARGETS_TO_BUILD=X86
devenv LLVM.sln /Build "Release|Win32" /project libclang
> Get libclang.lib from {somewhere}\llvm-3.7.1-build\Release\lib
> Get libclang.dll from {somewhere}\llvm-3.7.1-build\Release\bin

BUILD (x64)
===========
TODO

