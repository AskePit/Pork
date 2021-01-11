# Pork
Images/Video minimalistic viewer

# Build

I've only tried to build Pork on Windows systems so far.
Pork uses `vlc-qt` library, which in its turn uses `libvlc` library. So, the steps to build Pork are:

- Install Vlc player to your system. It'll give you libvlc dlls to your `C:\Program Files\VideoLAN\VLC` and `C:\Program Files (x86)\VideoLAN\VLC` paths (x86 and x64 respectively)
- Convert `libvlc.dll` and `libvlccore.dll` into lib files:
  - Run `dumpbin /exports your.dll`
  - Copy function names list. ex.:
```
      1    0 00001000 adler32
      2    1 00001350 adler32_combine
      3    2 00001510 compress
(etc.)
```
  - Convert this list to list like this:
```
EXPORTS
adler32
adler32_combine
compress
(etc.)
```

You can use regexp substitution for that task:
find: `\w+\s+\w+\s+\w+\s+(.*)`
replace: `$1`

  - Put this list to `.def` file
  - Run `lib /def:your.def /OUT:your.lib`
  - At the end you'll have `libvlc.lib` and `libvlccore.lib` files
- Build `vlc-qt` project refering `libvlc.lib` and `libvlccore.lib` libraries
- Use following files form `vlc-qt` `build` folder to build Pork:
  - `include`
  - `src/core/VLCQtCore.lib`
  - `src/core/Release/VLCQtCore.dll`
  - `src/widgets/VLCQtWidgets.lib`
  - `src/widgets/Release/VLCQtWidgets.dll`
- `libvlc.dll` and `libvlccore.dll` files are also needed while executing Pork application

Blame me, `vlc-qt` creator or `vlc` creators for such a shitty build adventure. Or contribute to the project and create convinient build system, that automates all routines above if you have a precious time.
