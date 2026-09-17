# FASTIO - Artax 4.1 JVS (Tomy Edition)

A community-driven fork of **derole1's fastio**, specifically tailored for **Artax 4.1 / Taito Type X** arcade systems. 

The **Tomy Edition** introduces dynamic configuration via a `fastio.ini` file, allowing arcade cabinet owners to adjust coin multipliers, input timings, and key bindings on the fly **without needing to recompile the C++ code**.

## ✨ Key Features
* **Dynamic Configuration (`fastio.ini`)**: Change settings instantly without touching the source code.
* **Coin Multiplier**: Insert 1 physical coin and inject multiple credits (e.g., 1 coin = 4 credits).
* **Adjustable Pulse Timing**: Fine-tune the keyboard injection speed (`PRESS_MS`) to fix dropped inputs in strict game engines (like SNK's *KOF XIII*).
* **SSD Protection**: Debug logging is disabled by default to prevent unnecessary read/write cycles on arcade SSDs.
* **Direct ScanCode Injection**: Uses hardware-level `SendInput` for maximum compatibility with arcade loaders.

## 🚀 Installation & Setup
1. Download the latest `fastio.exe` and `fastio.ini`.
2. Place both files in the same directory on your arcade system.
3. Edit `fastio.ini` with standard Notepad to match your setup.
4. Launch `fastio.exe` (or let your loader start it).

## ⚙️ Configuration (`fastio.ini`)
Here is the default configuration file:

```ini
[SYSTEM]
COM_PORT=COM2
DEBUG_LOG=0

[COIN_SETTINGS]
MULTIPLIER=4
PRESS_MS=50

[SYSTEM_KEYS]
COIN1=0x06
COIN2=0x07
TEST=0x08
SERVICE1=0x09
SERVICE2=0x0A
ESC=0x01
```

### Parameter Breakdown
* **`MULTIPLIER`**: Number of keystrokes sent per coin insertion.
* **`PRESS_MS`**: Duration (in milliseconds) of the key press and the pause between presses. 
  * *Default is `50`* (Works perfectly for games like *Under Night In-Birth*).
  * *Troubleshooting*: If a specific game (e.g., *KOF XIII*) misses credits, increase this to `60` or `75`.
* **`DEBUG_LOG`**: Set to `1` to generate a `cjvs.log` file for troubleshooting COM port issues. Set to `0` for normal use.
* **`COIN1` / `COIN2`**: The hexadecimal scancodes for Player 1 and Player 2 coin inputs (0x06 = Key '5', 0x07 = Key '6').

## 🛠️ Building from Source
If you wish to compile the project yourself:
1. Open the `.sln` or project file in **Visual Studio**.
2. Ensure you are targeting **x86** or **x64** depending on your specific Artax OS architecture.
3. Build the solution. The external `fastio.ini` file must be placed next to the newly compiled `.exe`.

---
*Based on the original JVS work by derole1. Modded for the Arcade Community.*
