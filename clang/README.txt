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
> MD llvm-3.7.1-x86
> CD llvm-3.7.1-x86
> cmake -G "Visual Studio 14" ..\llvm-3.7.1.src\ -DLLVM_TARGETS_TO_BUILD=X86
> devenv LLVM.sln /Build "Release|Win32" /project libclang
> Get libclang.lib from {somewhere}\llvm-3.7.1-x86\Release\lib
> Get libclang.dll from {somewhere}\llvm-3.7.1-x86\Release\bin

BUILD (x64)
===========
> Open Developer Command Prompt for Visual Studio
> CD {somewhere}
> MD llvm-3.7.1-x64
> CD llvm-3.7.1-x64
> cmake -G "Visual Studio 14 Win64" ..\llvm-3.7.1.src\ -DLLVM_TARGETS_TO_BUILD=X86
> devenv LLVM.sln /Build "Release|x64" /project libclang
> Get libclang.lib from {somewhere}\llvm-3.7.1-x64\Release\lib
> Get libclang.dll from {somewhere}\llvm-3.7.1-x64\Release\bin

