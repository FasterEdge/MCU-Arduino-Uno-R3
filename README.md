<div align="center">
<img src="https://avatars.githubusercontent.com/u/245985800?s=200&v=4" style="width:100px;" width="100"/>
<h2>FasterEdge MCU - Arduino Uno R3 (ATmega328P)</h2>
<h3>FasterEdge 框架的 Arduino Uno R3 平台实现（Arduino / PlatformIO 版）</h3>
</div>

### 一、简介

本项目是 **[FasterEdge](https://github.com/FasterEdge/FasterEdge)** 框架在 **Arduino Uno R3（ATmega328P）** 平台上的实现。ATmega328P 为 8 位 AVR 内核、2KB SRAM、32KB Flash、1KB 硬件 EEPROM，无网络、无操作系统，因此按 [MCU-C51](../MCU-C51) 的无网络精简思路裁剪能力子集，并保留 **寄存器 / GPIO / 芯片信息** 三个 MCU 专有模块。

- ✅ **arduino/（C++，Arduino 框架）** + **platformio_ide/（纯 C，寄存器级驱动）** 双版本
- ✅ 与主仓库**同名同命令**，云边协同对等编程
- ✅ HMAC-SHA256 纯 C 零依赖
- ✅ 配置/密钥持久化到内置 1KB 硬件 EEPROM
- ✅ platformio_ide 版为 **AVR 寄存器级真实实现**（USART0 / Timer0 / GPIO 端口）

### 二、已实现能力（无网络合理子集）

**Ability（8 个）**

| 名称 | 类别 | 命令 |
|------|------|------|
| `BaseAbility` | 基础 | `list_data_names` / `list_ability_names` |
| `RoleAbility` | 角色 | `describe` / `set_role` / `get_role` |
| `TimeAbility` | 时间 | `sync_manual` / `sync_system` / `get_time` / `configure_run`（无 NTP）|
| `OneKeyAbility` | 令牌 | `issue_token` / `verify_token` / `revoke_all` / `list_tokens` / `status` / `rotate`（HMAC-SHA256）|
| `SerialAbility` | 串口 | `open` / `close` / `write` / `read` / `is_open` / `set_config` / `get_config` / `list_ports` |
| `ModbusAbility` | Modbus | `set_unit_id` / `get_unit_id` / `read_holding` / `read_input` / `read_coils` / `read_discrete` / `write_holding` / `write_coil`（RTU 从站）|
| `RegAbility` | 寄存器(专有) | `read <addr>` / `write <addr>,<value>` / `bit_set <addr>,<bit>` / `bit_clear <addr>,<bit>` / `info` |
| `GpioAbility` | GPIO(专有) | `mode <pin>,<input\|output\|input_pullup>` / `write <pin>,<0\|1>` / `read <pin>` / `info` |

**Data（3 个）**

| 名称 | 功能 | 命令 |
|------|------|------|
| `BaseData` | 框架元信息 | `logo` / `info` |
| `ConfigData` | KV 配置（EEPROM 持久化）| `get` / `set` / `delete` / `list` / `snapshot` |
| `ChipData` | 芯片信息(专有) | `info` |

### 三、排除项与理由

| 能力 | 排除原因 |
|------|---------|
| MQTTAbility / NetMapData | ATmega328P 无网络协议栈 |
| EdgeRoleAbility | 依赖网络心跳上报 |
| ConfigFileAbility | 与 ConfigData 重复，且无文件系统概念 |
| KeyringData | 与 OneKeyAbility 合并（同一 EEPROM 密钥存储）|
| TimeAbility.sync_ntp | 无网络无法 SNTP 校时 |

### 四、目录结构

```
MCU-Arduino-Uno-R3/
├── arduino/                    # Arduino C++ 版（Arduino 框架）
│   ├── include/                # fe.h / fe_ability.h / fe_data.h / fe_hmac_sha256.h
│   ├── src/                    # fe.cpp / main.cpp / register.cpp / ability_*.cpp / data_*.cpp
│   └── platformio.ini          # board = uno（atmelavr + arduino framework）
└── platformio_ide/             # VS Code + PlatformIO IDE 工程（纯 C 寄存器级驱动）
    ├── platformio.ini          # board = uno（atmelavr + arduino framework）
    ├── .vscode/extensions.json # 推荐 PlatformIO IDE 插件
    ├── include/                # fe.h / fe_ability.h / fe_data.h / fe_port.h / fe_hmac_sha256.h
    └── src/                    # 裸机 C + AVR 寄存器级 fe_port（USART0 / Timer0 / GPIO / EEPROM）
```

> `arduino/` 与 `platformio_ide/` 能力与命令完全一致；前者面向 Arduino IDE / PlatformIO 快速上手，后者演示寄存器级裸机驱动，便于移植到其他 AVR（Nano/Mega）。

### 五、使用说明

1. **arduino 版**：用 Arduino IDE 打开 `arduino/src/main.cpp`（或 VS Code + PlatformIO 打开 `arduino/`），板卡选 **Arduino Uno**，烧录后串口 115200
2. **platformio_ide 版**：VS Code 安装 **PlatformIO IDE** 插件，打开 `platformio_ide/` 目录，底部状态栏 Build / Upload / Serial Monitor
3. 无需任何移植即可运行（fe_port 已实现 AVR 寄存器级驱动）

**串口命令示例：**

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

### 六、平台适配要点

| 差异点 | ESP32/ESP8266 | Uno R3 (ATmega328P) |
|--------|--------------|---------------------|
| 架构 | Xtensa 32 位 | **AVR 8 位** |
| RAM / Flash | KB~MB | **2KB / 32KB** |
| 存储 | NVS / Flash | **内置 1KB 硬件 EEPROM** |
| 网络 | 有 | **无**（能力子集剔除网络项）|
| 寄存器 | 32 位内存映射 | **8 位 I/O 空间 0x00-0xFF**（RegAbility 宽度 8）|

### 六-b、platformio_ide 版实现说明（AVR 寄存器级）

`platformio_ide/` 版不依赖 Arduino 库，`fe_port.c` 直接操作 ATmega328P 外设寄存器：

| 功能 | 实现 |
|------|------|
| UART | **USART0**（UBRR0 波特率 / UCSR0A/B/C / UDR0，RX 中断环形缓冲）|
| EEPROM | `avr/eeprom.h`（`eeprom_read_byte` / `eeprom_write_byte`，1KB）|
| 时间 | **Timer0 CTC 1ms 中断**计数（`TIMER0_COMPA_vect`）|
| GPIO | `DDRx / PORTx / PINx` + Arduino 引脚映射表（D0-D13 + A0-A5=14-19）|
| 随机数 | LCG（可扩展 ADC 噪声熵源）|

```bash
cd platformio_ide
pio run            # 编译
pio run -t upload  # 烧录
pio device monitor # 串口监视（115200）
```

> 换芯片：编辑 `platformio.ini` 的 `board`（如 `nanoatmega328`、`megaatmega2560`），并在 `fe_port.c` 中调整引脚映射表。

### 六-c、MCU 专有模块

除主仓库对应能力外，本仓库提供 3 个 **MCU 专有** 模块（寄存器 / GPIO / 芯片信息）。R3 的寄存器为 **8 位 AVR I/O 空间**（0x00-0xFF），GPIO 为 Arduino 引脚号：

| 模块 | 类型 | 命令 | 说明 |
|------|------|------|------|
| RegAbility | Ability | `read <addr>` / `write <addr>,<value>` / `bit_set <addr>,<bit>` / `bit_clear <addr>,<bit>` / `info` | AVR 内存映射寄存器（8 位，volatile 指针）|
| GpioAbility | Ability | `mode <pin>,<input\|output\|input_pullup>` / `write <pin>,<0\|1>` / `read <pin>` / `info` | Arduino 引脚 GPIO（pin 0-19）|
| ChipData | Data | `info` | ATmega328P 型号 / RAM / Flash / EEPROM / 频率 |

**示例：**

```
ability_RegAbility read 0x25          # 读 PORTB（数据空间地址）
ability_RegAbility write 0x25,0xAA    # 写 PORTB
ability_RegAbility bit_set 0x25,3
ability_GpioAbility mode 13,output    # 板载 LED
ability_GpioAbility write 13,1
ability_GpioAbility read 2
data_ChipData info
```

> ⚠️ 寄存器操作直接访问硬件，误写可能导致系统异常，仅供调试/底层驱动使用。

### 七、与 FasterEdge 主仓库的对应关系

- 命令名与主仓库**完全一致**，与 MCU-C51 / MCU-ESP32 实现同构
- `Atom` 模型：单例全局 Atom，`data_` / `ability_` 前缀路由
- 令牌用 HMAC-SHA256（纯 C，无 mbedTLS），密钥 EEPROM 持久化
- Modbus 寄存器表存 RAM，RTU 帧服务入口 `modbus_slave_service()` 已预留

### 八、姊妹项目

- **[FasterEdge MCU - ESP32](https://github.com/FasterEdge/MCU-ESP32)**：双核、WiFi/BLE、更多外设
- **[FasterEdge MCU - ESP8266](https://github.com/FasterEdge/MCU-ESP8266)**：WiFi、低功耗
- **[FasterEdge MCU - C51](https://github.com/FasterEdge/MCU-C51)**：8 位 8051，最精简
- **[FasterEdge MCU - Arduino Uno R4](https://github.com/FasterEdge/MCU-Arduino-Uno-R4)**：32 位 Cortex-M4F（RA4M1）
- **[FasterEdge](https://github.com/FasterEdge/FasterEdge)**：框架主仓库
