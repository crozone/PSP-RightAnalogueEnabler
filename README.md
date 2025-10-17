# PSP-RightAnalogueEnabler

PSP CFW PRX Plugin - NID patcher to enable right analogue stick data in `sceCtrl*Buffer*()` functions.

## Details

This is a Playstation Portable CFW plugin that patches the `sceCtrl*Buffer*()` controller read functions to include right analogue stick data, i.e. `SceCtrlData.rX` and `SceCtrlData.rY`.

It sources right analogue data from the DS3 controller port (port 1) as well as port 2 (officially unused).

## Why

When the PS Vita is emulating PSP games, the standard controller read functions return the right analogue stick rX, rY data in the standard `SceCtrlData` struct, as `SceCtrlData.rX` and `SceCtrlData.rY`.

This feature has been used to develop dual-analogue patches for several games, the most notable being the [theFlow Remastered Controls Collection](https://github.com/TheOfficialFloW/RemasteredControls). 

However, the PSP and PSP Go do not set these values in `SceCtrlData`. Instead, the `SceCtrlData2` functions must be used to explicitly read from the DS3 port to get the extra controller data. This is incompatible with the dual-analogue patches that were developed for the PS Vita.

To fix this, the read functions can be patched so that `SceCtrlData` is always populated with the right analogue data from the other controller ports.

\[UNCONFIRMED\] This may also enable right analogue input to work with POPS PS1 emulated games on PSP-1000/2000/3000/Non-Go. Currently, only PSP Go POPS is programmed to read DS3 `SceCtrlData2` data.

## Functions/NIDs patched

`sceController_Service`, `sceCtrl`:

* 0x3A622550 -> `s32 sceCtrlPeekBufferPositive(SceCtrlData *pData, u8 nBufs)`
* 0xC152080A -> `s32 sceCtrlPeekBufferNegative(SceCtrlData *pData, u8 nBufs)`
* 0x1F803938 -> `s32 sceCtrlReadBufferPositive(SceCtrlData *pData, u8 nBufs)`
* 0x60B81F86 -> `s32 sceCtrlReadBufferNegative(SceCtrlData *pData, u8 nBufs)`

## Installation

* You will need a custom firmware installed on your PSP. See the [ARK-4 project](github.com/PSP-Archive/ARK-4) for details on how to install it.
* Copy right_analogue_enabler.prx into /SEPLUGINS/ on the root of your Memory Stick
* Edit `SEPLUGINS/PLUGINS.TXT`
  * To run the plugin in games, add the line
    ```
    game, ms0:/SEPLUGINS/right_analogue_enabler.prx, on
    ```
  * See [ARK-4 Plugins](https://github.com/PSP-Archive/ARK-4/wiki/Plugins) for more details. 
* Restart the PSP.

## Building

* Follow the PSPDEV toolchain [installation steps](https://pspdev.github.io/installation.html)

### For release

```bash
mkdir -p -- build/release
cd build/release
psp-cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```

### For debug (enables Kprintf logging)

```bash
mkdir -p -- build/debug
cd build/debug
psp-cmake -DCMAKE_BUILD_TYPE=Debug ../..
make
```

## Debugging

The debug build .prx logs debug messages via `Kprintf()`.

Use [PSPLINK](https://github.com/pspdev/psplinkusb) to debug, with the appropriate [kprintf tool](https://github.com/pspdev/psplinkusb/tree/master/tools) to enable debug output.

* scrkprintf.prx: Print to display
* usbkprintf.prx: Print over USB (routes through printf)
* siokprintf.prx: Print to the serial port in the PSP headphone remote connector.