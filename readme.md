chuni-touch
---

`chuni-touch` adds touchscreen support for Chunithm. It works by hooking `CreateWindowExA()` and `CreateWindowExW()` and do `RegisterTouchWindow()` on every window created by Chunithm (with `ctw.dll`). It then takes over Chunithm's `WindowProc` and handles the touch input there (with `chuniio.dll`).

### Installation

`chuni-touch` requires `segatools` to work. If you are using some other tools to launch Chunithm, you are on your own.

If you are using `segatools`:

1. Download `chuni-touch` from the [release](https://github.com/Nat-Lab/chunithm-touch/releases) page.
2. Unzip `chuni-touch`, you will find `ctw.dll` and `chuniio.dll`.
3. Copy them to the `bin` folder of your game. Replace `chuniio.dll` if it already exists. 
4. Open `start.bat` with any text editor. Find the line `inject -d -k chunihook.dll chuniApp.exe`.
5. Append `-k ctw.dll` after `-k chunihook.dll`, so the line looks like this: `inject -d -k chunihook.dll -k ctw.dll chuniApp.exe`.

### Usage

Just tap and slide on the screen. Slide up to simulate the IR sensor. Use F1, F2, and F3 for test, service, and to insert coin.

### Configuration

Settings will be read from `chunitouch.ini`. Here's a list of configurable options:

```
[ir]
; height of each IR sensor
height = 50
; IR trigger threshold (number of pixels required to move up for a move to be registered as air)
trigger = 70

[slider]
; slider's width
width = 40
; slider's x-offset (pixels from the left of the screen)
offset = 318

[io]
; use raw input
raw_input = 0
```

### Building

`chuni-touch` uses the Meson build system. Install Meson and a recent build of MinGW-w64, then:

```
$ meson --cross cross-build-32.txt build32
$ ninja -C build32
$ meson --cross cross-build-64.txt build64
$ ninja -C build64
```

### License
UNLICENSE