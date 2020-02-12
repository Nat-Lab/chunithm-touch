chuni-touch
---

`chuni-touch` adds touchscreen support for Chunithm. It works by hooking `CreateWindowExA()` and `CreateWindowExW()` and do `RegisterTouchWindow()` on every window created by Chunithm (with `ctw.dll`). It then takes over Chunithm's `WindowProc` and handles the touch input there (with `chuniio.dll`).

### Installation

`chuni-touch` requires `segatools` to work. If you are using some other tools to launch Chunithm, you are on your own.

If you are using `segatools`:

1. Download `chuni-touch.zip` from the [release](https://github.com/Nat-Lab/chunithm-touch/releases) page.
2. Unzip `chuni-touch.zip`, you will find `ctw.dll` and `chuniio.dll`.
3. Copy them to the `bin` folder of your game. Replace `chuniio.dll` if it already exists. 
4. Open `start.bat` with any text editor. Find the line `inject -d -k chunihook.dll chuniApp.exe`.
5. Append `-k ctw.dll` after `-k chunihook.dll`, so the line looks like this: `inject -d -k chunihook.dll -k ctw.dll chuniApp.exe`.

### Usage

Just tap/slide on the screed and slide up to simulate the IR sensor. A video demo of how touch controls work is available [here](https://youtu.be/Uknwet_-wWw). Use F1, F2, and F3 for test, service, and to insert coin.

### Configuration

Settings will be read from `chunitouch.ini`. Here's a list of configurable options:

```
[ir]
; source of control. 'touch' for touchscreen and 'leap' for leap motion.
control_source = touch
; height of each touch IR sensor
touch_height = 50
; touch IR trigger threshold (number of pixels required to move up for a move to
; be registered as air)
touch_trigger = 70
; specifies the axis to track hands on. x, y, or z.
leap_orientation = y
; the minimum height of your hand(s) need to be for it to be registered as air
; (unit: millimeters)
leap_trigger = 500
; the height of each virtual IR sensor (unit: millimeters)
leap_step = 300

[slider]
; slider's width
width = 40
; slider's x-offset (pixels from the left of the screen)
offset = 318

[io]
; use raw input
raw_input = 0
; show Windows touch feedback
touch_feedback = 0
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