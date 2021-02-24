# Quest App Patcher

This program is a CLI tool that allows you to patch any app on your quest automatically to allow it to load mods, this is done through injecting 2 libs into the apk, signing it and reinstalling it, all done through command line interfacing in c++

To use this, you must provide the following executables when you clone this repo:

 - [apktool](https://ibotpeaches.github.io/Apktool/), expected is ver 2.5.0
 - [uber-apk-signer](https://github.com/patrickfav/uber-apk-signer) expected is ver 1.2.1
 - [adb](https://adbdownload.com/)
 
Also needed are a working java install and a way to compile the code obviously

# Patching
when all these executables are present in the root of the repository you can compile the program with the makefile and run the patcher.
make sure the app you want to install is actually installed on your quest and that you are actually connected with adb to your quest.

To compile the program I use mingw, which can be installed with chocolatey on windows (`choco install mingw`), which implements the g++ command for compiling on windows.

These instructions are publicly available, but that doesn't mean they are useful for the general public. 
For application specific patching I recommend editing the program so it only checks for that application. You are free to do so, as long as you credit me for the original implementation
