<div align="center">
<img src="https://avatars.githubusercontent.com/u/245985800?s=200&v=4" style="width:100px;" width="100"/>
<h2>FasterEdge MCU - Arduino Uno R3 (ATmega328P)</h2>
<h3>FasterEdge framework on Arduino Uno R3 (Arduino / PlatformIO editions)</h3>
</div>

### 1. Introduction

This repo implements the **[FasterEdge](https://github.com/FasterEdge/FasterEdge)** framework on the **Arduino Uno R3 (ATmega328P)**. The ATmega328P is an 8-bit AVR core with 2KB SRAM, 32KB Flash and 1KB on-chip EEPROM — no network, no OS. Following the [MCU-C51](../MCU-C51) no-network design, the capability set is trimmed and 3 **MCU-specific** modules (registers / GPIO / chip info) are kept.

- ✅ **arduino/ (C++, Arduino framework)** + **platformio_ide/ (pure C, register-level drivers)** dual editions
- ✅ Same names & commands as the main repo — peer programming for edge/cloud
- ✅ HMAC-SHA256 in pure C (zero dependencies)
- ✅ Config/keys persisted to the 1KB on-chip EEPROM
- ✅ platformio_ide edition ships a **real AVR register-level implementation** (USART0 / Timer0 / GPIO ports)

### 2. Implemented Capabilities (no-network subset)

**Abilities (8)**

| Name | Type | Commands |
|------|------|----------|
| `BaseAbility` | Base | `list_data_names` / `list_ability_names` |
| `RoleAbility` | Role | `describe` / `set_role` / `get_role` |
| `TimeAbility` | Time | `sync_manual` / `sync_system` / `get_time` / `configure_run` (no NTP) |
| `OneKeyAbility` | Token | `issue_token` / `verify_token` / `revoke_all` / `list_tokens` / `status` / `rotate` (HMAC-SHA256) |
| `SerialAbility` | Serial | `open` / `close` / `write` / `read` / `is_open` / `set_config` / `get_config` / `list_ports` |
| `ModbusAbility` | Modbus | `set_unit_id` / `get_unit_id` / `read_holding` / `read_input` / `read_coils` / `read_discrete` / `write_holding` / `write_coil` (RTU slave) |
| `RegAbility` | Reg (own) | `read <addr>` / `write <addr>,<value>` / `bit_set <addr>,<bit>` / `bit_clear <addr>,<bit>` / `info` |
| `GpioAbility` | GPIO (own) | `mode <pin>,<input|output|input_pullup>` / `write <pin>,<0|1>` / `read <pin>` / `info` |

**Data (3)**

| Name | Type | Commands |
|------|------|----------|
| `BaseData` | Meta | `logo` / `info` |
| `ConfigData` | KV config (EEPROM) | `get` / `set` / `delete` / `list` / `snapshot` |
| `ChipData` | Chip info (own) | `info` |

### 3. Excluded Capabilities

| Capability | Reason |
|------------|--------|
| MQTTAbility / NetMapData | No network stack on ATmega328P |
| EdgeRoleAbility | Needs network heartbeat |
| ConfigFileAbility | Redundant with ConfigData; no filesystem concept |
| KeyringData | Merged into OneKeyAbility (same EEPROM key) |
| TimeAbility.sync_ntp | No network for SNTP |

### 4. Directory Layout

```
MCU-Arduino-Uno-R3/
├── arduino/                    # Arduino C++ edition (Arduino framework)
│   ├── include/                # fe.h / fe_ability.h / fe_data.h / fe_hmac_sha256.h
│   ├── src/                    # fe.cpp / main.cpp / register.cpp / ability_*.cpp / data_*.cpp
│   └── platformio.ini          # board = uno (atmelavr + arduino framework)
└── platformio_ide/             # VS Code + PlatformIO IDE project (pure C register-level)
    ├── platformio.ini          # board = uno (atmelavr + arduino framework)
    ├── .vscode/extensions.json # recommends PlatformIO IDE
    ├── include/                # fe.h / fe_ability.h / fe_data.h / fe_port.h / fe_hmac_sha256.h
    └── src/                    # bare-metal C + AVR register-level fe_port (USART0 / Timer0 / GPIO / EEPROM)
```

> `arduino/` and `platformio_ide/` expose identical capabilities & commands; the former is for quick start, the latter demonstrates register-level bare-metal drivers and eases porting to other AVRs (Nano/Mega).

### 5. Usage

1. **arduino edition**: open `arduino/src/main.cpp` in Arduino IDE (board: Arduino Uno), or open `arduino/` in VS Code + PlatformIO; flash and monitor at 115200.
2. **platformio_ide edition**: install the **PlatformIO IDE** VS Code extension, open `platformio_ide/`, use Build / Upload / Serial Monitor from the status bar.
3. No porting needed — `fe_port.c` already ships real AVR register-level drivers.

**Serial command examples:**

```
help
ability_BaseAbility list_ability_names
ability_RoleAbility set_role edge
ability_TimeAbility sync_manual 1700000000
ability_OneKeyAbility issue_token sensor01
ability_ModbusAbility set_unit_id 3
ability_ModbusAbility write_holding 0,42
ability_ModbusAbility read_holding 0,4
ability_SerialAbility set_config 0,9600
ability_SerialAbility write hello
data_ConfigData set wifi.ssid=MyNet
data_ConfigData get wifi.ssid
data_BaseData info
```

### 6. Platform Differences

| Aspect | ESP32/ESP8266 | Uno R3 (ATmega328P) |
|--------|---------------|---------------------|
| Architecture | Xtensa 32-bit | **AVR 8-bit** |
| RAM / Flash | KB~MB | **2KB / 32KB** |
| Storage | NVS / Flash | **1KB on-chip EEPROM** |
| Network | Yes | **No** (network items trimmed) |
| Registers | 32-bit MMIO | **8-bit I/O space 0x00-0xFF** (RegAbility width 8) |

### 6-b. platformio_ide Notes (AVR register-level)

The `platformio_ide/` edition does not depend on Arduino libraries; `fe_port.c` drives the ATmega328P peripherals directly:

| Function | Implementation |
|----------|----------------|
| UART | **USART0** (UBRR0 baud / UCSR0A/B/C / UDR0, RX ISR ring buffer) |
| EEPROM | `avr/eeprom.h` (`eeprom_read_byte` / `eeprom_write_byte`, 1KB) |
| Time | **Timer0 CTC 1ms ISR** counter (`TIMER0_COMPA_vect`) |
| GPIO | `DDRx / PORTx / PINx` + Arduino pin map (D0-D13 + A0-A5=14-19) |
| Random | LCG (ADC-noise entropy possible) |

```bash
cd platformio_ide
pio run            # build
pio run -t upload  # flash
pio device monitor # serial monitor (115200)
```

> To change MCU: edit `board` in `platformio.ini` (e.g. `nanoatmega328`, `megaatmega2560`) and adjust the pin map in `fe_port.c`.

### 6-c. MCU-Specific Modules

Beyond main-repo capabilities, 3 **MCU-specific** modules (registers / GPIO / chip info) are provided. The R3 registers are **8-bit AVR I/O space** (0x00-0xFF); GPIO uses Arduino pin numbers:

| Module | Type | Commands | Description |
|--------|------|----------|-------------|
| RegAbility | Ability | `read <addr>` / `write <addr>,<value>` / `bit_set <addr>,<bit>` / `bit_clear <addr>,<bit>` / `info` | AVR MMIO registers (8-bit, volatile pointer) |
| GpioAbility | Ability | `mode <pin>,<input|output|input_pullup>` / `write <pin>,<0|1>` / `read <pin>` / `info` | Arduino pin GPIO (pin 0-19) |
| ChipData | Data | `info` | ATmega328P model / RAM / Flash / EEPROM / freq |

**Examples:**

```
ability_RegAbility read 0x25          # read PORTB (data-space addr)
ability_RegAbility write 0x25,0xAA    # write PORTB
ability_RegAbility bit_set 0x25,3
ability_GpioAbility mode 13,output    # onboard LED
ability_GpioAbility write 13,1
ability_GpioAbility read 2
data_ChipData info
```

> ⚠️ Register access touches hardware directly; a wrong write may crash the system. Debug/low-level use only.

### 7. Correspondence with the Main Repo

- Commands match the main repo exactly, and the implementation is isomorphic with MCU-C51 / MCU-ESP32.
- `Atom` model: singleton global Atom, `data_` / `ability_` prefix routing.
- Tokens via HMAC-SHA256 (pure C, no mbedTLS), key persisted in EEPROM.
- Modbus register tables live in RAM; RTU entry `modbus_slave_service()` is reserved.

### 8. Sibling Projects

- **[FasterEdge MCU - ESP32](https://github.com/FasterEdge/MCU-ESP32)**: dual-core, WiFi/BLE, more peripherals
- **[FasterEdge MCU - ESP8266](https://github.com/FasterEdge/MCU-ESP8266)**: WiFi, low power
- **[FasterEdge MCU - C51](https://github.com/FasterEdge/MCU-C51)**: 8-bit 8051, most minimal
- **[FasterEdge MCU - Arduino Uno R4](https://github.com/FasterEdge/MCU-Arduino-Uno-R4)**: 32-bit Cortex-M4F (RA4M1)
- **[FasterEdge](https://github.com/FasterEdge/FasterEdge)**: framework main repo
