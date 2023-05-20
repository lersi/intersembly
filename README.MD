# INTERSEMBLY project

The interactive assembly shell

## About

This project gets assembly from the command line, assembles it and executes it in the same process! (no emulation and no sandboxing).


## Prerequires

This project depends on Cmake and Keystone library.

## Compile

use the following command in order to configure the project:  
`cmake -S . -B ./build -DTARGET_ARCH=[x64|aarch64]`

for compiling the code:  
`cmake --build ./build --target intersembly`
