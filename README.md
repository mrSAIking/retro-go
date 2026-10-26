<div align="center">

# ESP32-S3 Retro Gaming Console

### Custom Retro-Go Port · ST7735 TFT · microSD · MAX98357A I2S Audio

**A DIY retro gaming device powered by the ESP32-S3 N16R8, with physical controls, removable game storage, and working speaker audio.**

![ESP32-S3](https://img.shields.io/badge/ESP32--S3-N16R8-111111?style=flat-square&labelColor=000000)
![Display](https://img.shields.io/badge/Display-ST7735_160x128-111111?style=flat-square&labelColor=000000)
![Audio](https://img.shields.io/badge/Audio-MAX98357A_I2S-111111?style=flat-square&labelColor=000000)
![Build](https://img.shields.io/badge/Build-GitHub_Actions-111111?style=flat-square&labelColor=000000)
![License](https://img.shields.io/badge/License-GPLv2-111111?style=flat-square&labelColor=000000)

*This section documents the custom ESP32-S3 implementation. The original Retro-Go documentation is preserved below.*

</div>

---

## Custom ESP32-S3 Build

This fork adds a custom target for an **ESP32-S3 N16R8** development board, a **1.8-inch ST7735 TFT**, an external **SPI microSD reader**, **eight active-LOW buttons**, and a **MAX98357A I2S amplifier and speaker**. The project was built using GitHub Actions rather than a local ESP-IDF installation.

### Verified project functionality

| Feature | Status |
|---|---|
| ESP32-S3 boots custom Retro-Go firmware | ✅ Working |
| Retro-Go graphical launcher | ✅ Working |
| ST7735 landscape display (160 × 128) | ✅ Working |
| Eight physical gaming buttons | ✅ Tested |
| SPI microSD ROM storage | ✅ Tested |
| DOOM gameplay (PrBoom-Go) | ✅ Working |
| MAX98357A I2S speaker audio | ✅ Working — sound confirmed on hardware |
| PlayStation 1 emulation | Not included in this build |
| Java / J2ME games | Not included in this build |

**Scope:** These are observations for this custom device, not claims that every emulator listed in the original Retro-Go project has been tested or included in this particular firmware image.

### Hardware

| Component | Specification |
|---|---|
| Microcontroller | ESP32-S3 N16R8 |
| Flash memory | 16 MB |
| PSRAM | 8 MB |
| Screen | 1.8-inch ST7735 SPI TFT |
| Resolution | 160 × 128, landscape |
| Storage | External SPI microSD module; tested using a 4 GB microSD card |
| Input | 8 GPIO push buttons with internal pull-ups |
| Audio | MAX98357A I2S amplifier and connected speaker — working |
| Firmware | Custom Retro-Go target built with ESP-IDF 4.4.8 |

### Pin assignments

These pin mappings match [`components/retro-go/targets/esp32-s3-devkit/config.h`](components/retro-go/targets/esp32-s3-devkit/config.h) in this fork.

#### ST7735 TFT (SPI2_HOST, 20 MHz)

| Display signal | ESP32-S3 pin |
|---|---|
| MOSI / SDA | GPIO 21 |
| SCK / SCL | GPIO 18 |
| CS | GPIO 5 |
| DC / A0 | GPIO 4 |
| RST / RESET | GPIO 3 |
| LED / backlight | 3.3 V (always on; not GPIO controlled) |
| MISO | Not connected |

Screen settings: `RG_SCREEN_DRIVER=2`, `RG_SCREEN_WIDTH=160`, `RG_SCREEN_HEIGHT=128`, `RG_SCREEN_ROTATE=0`, `RG_ST7735_X_OFFSET=0`, `RG_ST7735_Y_OFFSET=0`.

#### SPI microSD module (SPI3_HOST)

| microSD signal | ESP32-S3 pin |
|---|---|
| MISO | GPIO 10 |
| MOSI | GPIO 11 |
| CLK / SCK | GPIO 12 |
| CS | GPIO 13 |

The firmware mounts external storage at `/sd`.

#### Eight gaming buttons

| Button | ESP32-S3 pin |
|---|---|
| UP | GPIO 6 |
| DOWN | GPIO 7 |
| LEFT | GPIO 8 |
| RIGHT | GPIO 9 |
| A | GPIO 15 |
| B | GPIO 16 |
| START | GPIO 17 |
| SELECT | GPIO 14 |

Buttons use **active LOW** wiring with internal pull-up resistors. The target maps **START + SELECT** to `MENU`, and **SELECT + A** to `OPTION`.

#### MAX98357A I2S amplifier — tested and working

| MAX98357A signal | ESP32-S3 pin |
|---|---|
| BCLK / BCK | GPIO 41 |
| LRC / WS | GPIO 42 |
| DIN | GPIO 40 |

The MAX98357A amplifier is **physically connected to a speaker**, and **speaker sound output was confirmed working**. The firmware uses external I2S audio (`RG_AUDIO_USE_EXT_DAC=1`) and disables the internal DAC (`RG_AUDIO_USE_INT_DAC=0`). Power, ground, and speaker terminals must be wired appropriately for the particular MAX98357A module; follow the module's documentation rather than treating signal GPIOs as power pins.

### Firmware and source

- Hardware target: [`esp32-s3-devkit`](components/retro-go/targets/esp32-s3-devkit/)
- Target configuration: [`config.h`](components/retro-go/targets/esp32-s3-devkit/config.h)
- Build environment override: [`env.py`](components/retro-go/targets/esp32-s3-devkit/env.py), setting `IDF_TARGET="esp32s3"` and `FW_FORMAT="none"`
- Cloud build: [`.github/workflows/retro-go-build.yml`](.github/workflows/retro-go-build.yml)

The automated complete-image workflow builds these applications:

| Application | Purpose |
|---|---|
| `launcher` | Game launcher and user interface |
| `retro-core` | Retro emulation application |
| `prboom-go` | DOOM engine |

The wider Retro-Go source tree contains other emulator applications, but they are **not automatically part of this complete-image build**.

### Build firmware using GitHub Actions

1. Open the repository's **Actions** tab.
2. Select **Retro-Go ESP32-S3 Complete Firmware**.
3. Select **Run workflow** on the intended branch.
4. Once the build succeeds, download the artifact named `retro-go-esp32s3-complete`.
5. Extract it and locate the **complete `.img` file**. The artifact also contains separate application binaries, a partition table, a manifest, and flashing instructions.

The workflow runs on **Ubuntu 22.04**, installs **ESP-IDF v4.4.8**, and builds for the **ESP32-S3**. The produced full image includes the bootloader, partition table, launcher, Retro-Core and DOOM application partitions.

### Flash the ESP32-S3

Connect the ESP32-S3 to a Windows computer with a USB data cable. From an environment with [esptool](https://github.com/espressif/esptool) installed, flash **the complete generated image at address `0x0`**:

```bash
python -m esptool --chip esp32s3 --port COM_PORT write_flash --flash_size detect 0x0 firmware.img
```

Replace `COM_PORT` with the actual serial port (for example, `COM5`) and `firmware.img` with the artifact's **actual image filename**. The syntax may differ across esptool versions; use the version-compatible command when necessary. **Do not flash the separate `.bin` files as though they were the complete image.**

### Add games to microSD

Use a FAT32-formatted microSD card and put **legally obtained** ROM files in the game folders expected by Retro-Go. A typical layout is:

```text
microSD/
└── roms/
    ├── nes/
    ├── gb/
    ├── gbc/
    ├── sms/
    └── doom/
        └── <compatible IWAD file>
```

DOOM requires a compatible **IWAD** in `/roms/doom/`. ROMs and copyrighted game data are **not included** in this repository or build artifact.

### Known limitations and notes

- The TFT is physically 128 × 160 but configured in **landscape 160 × 128** mode.
- The screen backlight is connected to 3.3 V, so firmware backlight dimming is not configured.
- No battery-level measurement is configured (`RG_BATTERY_DRIVER=0`).
- Emulator speed and ROM compatibility depend on the emulator and game. The upstream list below is a **project-wide capability list**, not a list of games verified on this custom hardware.
- The original Retro-Go documentation below also describes ODROID-GO-specific installation and audio hardware; those procedures **do not apply** to this ESP32-S3 device unless explicitly stated.

### Original project and attribution

This is a **custom hardware port/fork of [Retro-Go by Alex Duchesne (ducalex)](https://github.com/ducalex/retro-go)**, not a replacement for the original project. The original documentation, acknowledgements and license details are retained below. Please credit and comply with the upstream licenses when distributing modified firmware.

---

# Original Retro-Go Documentation

# Table of contents
- [Description](#description)
- [Installation](#installation)
- [Usage](#usage)
- [Issues](#issues)
- [Development](#development)
- [Acknowledgements](#acknowledgements)
- [License](#license)

# Description
Retro-Go is a firmware to play retro games on ESP32-based devices (officially supported are
ODROID-GO and MRGC-G32, check [this list for other devices](components/retro-go/README.md)).
The project consists of a launcher and half a dozen applications that have been heavily
optimized to reduce their cpu, memory, and flash needs without reducing compatibility!

### Supported systems:
- Nintendo: **NES, SNES (slow), Gameboy, Gameboy Color, Game & Watch**
- Sega: **SG-1000, Master System, Mega Drive / Genesis, Game Gear**
- Coleco: **Colecovision**
- NEC: **PC Engine**
- Atari: **Lynx**
- Others: **DOOM** (including mods!)

### Retro-Go features:
- In-game menu
- Favorites and recently played
- GB color palettes, RTC adjust and save
- NES color palettes, PAL roms, NSF support
- More emulators and applications
- Scaling and filtering options
- Better performance and compatibility
- Turbo Speed/Fast forward
- Customizable launcher
- Cover art and save state previews
- Multiple save slots per game
- Wifi file manager
- And more!

### Screenshots
![Preview](assets/retro-go-preview.jpg)


# Installation

### ODROID-GO
  1. Download `retro-go_1.x_odroid-go.fw` from the [release page](https://github.com/ducalex/retro-go/releases/) and copy it to `/odroid/firmware` on your sdcard.
  2. Power up the device while holding down B.
  3. Select retro-go in the files list and flash it.

### MyRetroGameCase G32 (GBC)
  1. Download `retro-go_1.x_mrgc-g32.fw` from the [release page](https://github.com/ducalex/retro-go/releases/) and copy it to `/espgbc/firmware` on your sdcard.
  2. Power up the device while holding down MENU (the volume knob).
  3. Select retro-go in the files list and flash it.

### Other devices
  1. Download the .img for your device from the [release page](https://github.com/ducalex/retro-go/releases/).
  2. Connect your device to a computer with a USB cable.
  3. Flash the image with esptool:
     - [Command line](https://github.com/espressif/esptool/releases/): Run `esptool.py write_flash --flash_size detect 0x0 retro-go_*.img`
     - [Web version](https://espressif.github.io/esptool-js/): Connect your device, click Erase Flash, then select your .img file and set address to 0x0, finally click Program)

Your particular device may require extra steps (like holding a button during power up) or different esptool flags or a special cable. If the above steps fail, you might need to ask the manufacturer for instructions on how to flash new firmware!

If your device is not already supported or if a prebuilt version isn't available for it you can check the [development section](#Development) for more information on how to build for your device.


# Usage

## Game covers / artwork
Game covers should be placed in the `romart` folder at the base of your sd card. You can obtain a pre-made pack [here](https://github.com/ducalex/retro-go-covers). Retro-Go is also compatible with the older Go-Play romart pack.

You can add missing cover art by creating a PNG image (160x168, 8bit). Two naming schemes are supported:
- Filename-based: `/romart/nes/Super Mario.png` (notice the rom extension is *not* included)
- CRC32-based: `/romart/nes/A/ABCDE123.png` where `nes` is the same as the rom folder, and `ABCDE123` is the CRC32 of the game (press A -> Properties in the launcher to find it), and `A` is the first character of the CRC32

_Note: CRC32-based, which is what is used in the pre-made pack, is much slower than name-based! This type is useful because filenames vary greatly despite having identical CRCs, but if you generate your own art I suggest you use filename-based format and delete all CRC-based art from your SD Card to improve responsiveness._


## BIOS files
Some emulators support loading a BIOS. The files should be placed as follows:
- GB: `/retro-go/bios/gb_bios.bin`
- GBC: `/retro-go/bios/gbc_bios.bin`
- FDS: `/retro-go/bios/fds_bios.bin`
- MSX: In folder `/retro-go/bios/msx/` put: `MSX.ROM` `MSX2.ROM` `MSX2EXT.ROM` `MSX2P.ROM` `MSX2PEXT.ROM` `FMPAC.ROM` `DISK.ROM` `MSXDOS2.ROM` `PAINTER.ROM` `KANJI.ROM`


## Game & Watch
The roms must be packed with [LCD-Game-Shrinker](https://github.com/bzhxx/LCD-Game-Shrinker) and a tutorial can be [found here](https://gist.github.com/DNA64/16fed499d6bd4c0a9638e4ef).


## Wifi
To use wifi you will need to create a `/retro-go/config/wifi.json` config file. You can define up to 4 different networks, then selectable in the menu. Its content should look like this:

````json
{
  "ssid0": "my-network",
  "password0": "my-password",
  "ssid1": "my-other-network",
  "password1": "my-password",
  "ssid2": "my-third-network",
  "password2": "my-password",
  "ssid3": "my-last-network",
  "password3": "my-password"
}
````

### Time synchronization
Time synchronization happens in the launcher immediately after a successful connection to the network.
This is done via NTP by contacting `pool.ntp.org` and cannot be disabled at this time.
Timezone can be configured in the launcher's options menu.

### File manager
You can find the IP of your device in the *about* menu of retro-go. Then on your PC navigate to
http://192.168.x.x/ to access the file manager.


## External DAC (headphones)

Retro-Go supports [the external DAC mod for the ODROID-GO](https://github.com/backofficeshow/odroid-go-audio-hat)
which allows high quality audio through headphones. You can switch to it in the menu `Audio Out: Ext DAC`.

<details>
  <summary>Pinout</summary>

  | GO PIN | PCM5102A PIN |
  |--------|---------|
  | 1 | GND |
  | 2 | - |
  | 3 | LCK |
  | 4 | DIN |
  | 5 | BCK |
  | 6 | VIN |
  | 7 | - |
  | 8 | - |
  | 9 | - |
  | 10 | - |
</details>


# Issues

### Black screen / Boot loops
Retro-Go typically detects and resolves application crashes and freezes automatically. However, if you do
get stuck in a boot loop, you can hold `DOWN` while powering up the device to return to the launcher.

### Sound quality
The volume isn't correctly attenuated on the GO, resulting in upper volume levels that are too loud and
lower levels that are distorted due to DAC resolution. A quick way to improve the audio is to cut one
of the speaker wire and add a `33 Ohm (or thereabout)` resistor in series. Soldering is better but not
required, twisting the wires tightly will work just fine.
[A more involved solution can be seen here.](https://wiki.odroid.com/odroid_go/silent_volume)
Alternatively you can use the headphones DAC mod mentioned earlier in this document.

### Game Boy SRAM *(aka Save/Battery/Backup RAM)*
In Retro-Go, save states will provide you with the best and most reliable save experience. That being said, please
read on if you need or want SRAM saves. The SRAM format is compatible with VisualBoyAdvance so it may be used to
import or export saves.

You can configure automatic SRAM saving in the options menu. A longer delay will reduce stuttering at the cost of
losing data when powering down too quickly. Also note that when *resuming* a game, Retro-Go will give priority
to a save state if present.

### ZIP files
Most Retro-Go applications now support ZIP files. ZIP archives should contain only one ROM file and nothing else. ZIP support also depends on available memory and larger ROMs may fail to load on some devices unfortunately.


# Development
If you wish to build or modify Retro-Go, you can find help in the following documents:

- Build instructions in [BUILDING.md](BUILDING.md)
- Theming instructions [THEMING.md](THEMING.md)
- Porting instructions in [PORTING.md](PORTING.md)
- Translating instructions in [LOCALIZATION.md](LOCALIZATION.md)


# Acknowledgements
- The NES/GBC/SMS emulators and base library were originally from the "Triforce" fork of the [official Go-Play firmware](https://github.com/othercrashoverride/go-play) by crashoverride, Nemo1984, and many others.
- The design of the launcher was originally inspired/copied from [pelle7's go-emu](https://github.com/pelle7/odroid-go-emu-launcher).
- PCE-GO is a fork of [HuExpress](https://github.com/kallisti5/huexpress) and [pelle7's port](https://github.com/pelle7/odroid-go-pcengine-huexpress/) was used as reference.
- The Lynx emulator is a port of [libretro-handy](https://github.com/libretro/libretro-handy).
- The SNES emulator is a port of [Snes9x 2005](https://github.com/libretro/snes9x2005).
- The DOOM engine is a port of [PrBoom 2.5.0](http://prboom.sourceforge.net/).
- The Genesis emulator is a port of [Gwenesis](https://github.com/bzhxx/gwenesis/) by bzhxx.
- The Game & Watch emulator is a port of [lcd-game-emulator](https://github.com/bzhxx/lcd-game-emulator) by bzhxx.
- The MSX emulator is a port of [fMSX](https://fms.komkon.org/fMSX/) by Marat Fayzullin.
- PNG support is provided by [lodepng](https://github.com/lvandeve/lodepng/).
- PCE cover art is from [Christian_Haitian](https://github.com/christianhaitian).
- Some icons from [Rokey](https://iconarchive.com/show/seed-icons-by-rokey.html).
- Background images from [es-theme-gbz35](https://github.com/rxbrad/es-theme-gbz35).
- Special thanks to [RGHandhelds](https://www.rghandhelds.com/) and [MyRetroGamecase](https://www.myretrogamecase.com/) for sending me a [G32](https://www.myretrogamecase.com/products/game-mini-g32-esp32-retro-gaming-console-1) device.
- The [ODROID-GO](https://forum.odroid.com/viewtopic.php?f=159&t=37599) community for encouraging the development of retro-go!

# License
Everything in this project is licensed under the [GPLv2 license](COPYING) with the exception of the following components:
- fmsx/components/fmsx (MSX Emulator, custom non-commercial license)
- handy-go/components/handy (Lynx emulator, zlib)
