chuni-touch
---

`chuni-touch` adds touchscreen support and Leap Motion for Chunithm. For touch, It works by hooking `CreateWindowExA()` and `CreateWindowExW()` and do `RegisterTouchWindow()` on every window created by Chunithm (with `ctw.dll`). It then takes over Chunithm's `WindowProc` and handles the touch input there (with `chuniio.dll`).

### Installation

`chuni-touch` requires `segatools` to work. If you are using some other tools to launch Chunithm, you are on your own.

If you are using `segatools`:

1. Download `chuni-touch.zip` from the [release](https://github.com/Nat-Lab/chunithm-touch/releases) page.
2. Unzip `chuni-touch.zip`, copy everything in it to the `bin` folder of your game. Override any file that already exists. You may want to make a backup of your `bin` folder.
3. (Optional) If you plan to use a Leap Motion for AIR and AIR-actions, run `leapconfig.exe` to configure your Leap Motion controller. You may configure the controller manually in `chunitouch.ini` too.

### Usage

Just tap/slide on the screen. Slide up to simulate the IR sensor if you are using touch-bashed IR simulation. Raise your hand as if you were playing on the real arcade to simulate the IR sensor if you are using a Leap controller. A video demo of how touch controls work is available [here](https://youtu.be/Uknwet_-wWw). Use F1, F2, and F3 for test, service, and to insert coin. 

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

[misc]
; keep slider(s) holded while on air-action
ir_keep_slider = 0
```

### Building

To build chuni-touch, you will need to get the Leap Motion standard Orion SDK package from [Leap Motion developer site](http://developer.leapmotion.com). Unzip the SDK package and copy the `LeapSDK` folder to `3rdparty/`.

You may build `chuni-touch` on with any operating system that can run MinGW-w64. On Windows:

```
> meson build
> ninja -C build
```

On Unix-like: 

```
$ meson --cross cross-build-32.txt build32
$ ninja -C build32
$ meson --cross cross-build-64.txt build64
$ ninja -C build64
```

Or, if you are using Windows and have Visual Studio installed, you may build it with Visual Studio: 

```
> meson --backend vs build
> msbuild build\chunithm-touch.sln
```

### License
UNLICENSE