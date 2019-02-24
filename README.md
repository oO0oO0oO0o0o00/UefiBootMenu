# UefiBootMenu

A graphic uefi boot menu as application ( post Dxe only )

炒鸡水的作业...

![Screenshot](arts/scr01.png?raw=true)

## Build

Oh really? Are you serious?

1. Clone/Download UDK2018 & get ready, from [Here](https://github.com/tianocore/tianocore.github.io/wiki/UDK2018-How-to-Build#how-to-build-windows-system).
2. Build emulator. For windows: 
```cmd
  edksetup.bat --nt32
  build -a X64 -t VS2017 -p Nt32Pkg/Nt32Pkg.dsc
```
3. Create shortcut to emulator, say `efi.bat` on Desktop:
```cmd
[Replace with full path to workspace]\Build\NT32X64\DEBUG_VS2017\X64\SecMain.exe
pause
```
4. Clone this project. From edk workspace (with lots of `xxxPkg`s) run:
```cmd
git clone https://github.com/oO0oO0oO0o0o00/UefiBootMenu.git
```
This will create a `MeowPkg`.

5. Build project. Make use of `Conf/target.txt` or just `build -a X64 -t VS2017 -p MeowPkg/MeowPkg.dsc`.
6. Create a script to copy generated file to emulator home, say `meow2emu.bat` in workspace root:
```cmd
@echo off
DEL /Q .\Build\NT32X64\DEBUG_VS2017\X64\Meow.efi
COPY .\Build\MeowPkg\DEBUG_VS2017\X64\MeowPkg\Meow\Meow\OUTPUT\Meow.efi .\Build\NT32X64\DEBUG_VS2017\X64\Meow.efi
```
7. Run it.
8. Start emulater, Enter, Esc, `fs0:`,`me`, Tab, Enter.

VirtualBox or Ovmf on Qemu are also good test platforms.

Would anyone look at this? If you're, hey fuck♂your♂ass~~~
