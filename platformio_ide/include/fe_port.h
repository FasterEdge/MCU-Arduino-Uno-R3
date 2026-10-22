// ─────────────────────────────────────────────────────────────
// FasterEdge 开源项目
// Github: https://github.com/FasterEdge
// Gitee:  https://gitee.com/FasterEdge
// ─────────────────────────────────────────────────────────────
// fe_port.h — FasterEdge MCU 平台移植层（Arduino Uno R3 / ATmega328P 版）
// 平台相关能力在此抽象：UART 收发、EEPROM 存储、系统时间、
// 随机数、GPIO 引脚控制、芯片信息。R3 无网络，不提供 WiFi/TCP。
// 移植到其他 AVR（Nano/Mega 等）时只需实现本文件。
#ifndef FE_PORT_H
#define FE_PORT_H

#include "fe.h"   // u8/u16/u32 类型、TRUE/FALSE

// ============================================================
// Flash 常量（AVR PROGMEM）与宿主编译兼容层
// ATmega328P 只有 2KB SRAM：字符串/常量表默认会被 avr-gcc
// 放进 .data 并占用 RAM，因此统一标注 FE_PROGMEM 并走
// pgm_read_* 读取；在宿主机测试/语法检查时退化为普通常量。
// ============================================================
#ifdef __AVR__
#include <avr/pgmspace.h>
#define FE_PROGMEM         PROGMEM
#define FE_PSTR(s)         PSTR(s)
#define FE_PGM_CHAR(p)     pgm_read_byte((PGM_P)(p))
#define FE_PGM_DWORD(p)    pgm_read_dword((PGM_P)(p))
#define FE_PGM_PTR(p)      ((const void *)pgm_read_word((PGM_P)(p)))
#define FE_PGM_CMD_NAME(cmds, c) ((const char *)FE_PGM_PTR(&(cmds)[c].name))
#define FE_PGM_EQ(a, s)    (strcmp_P((a), PSTR(s)) == 0)
#define FE_PGM_STRCMP(a, p) (strcmp_P((a), (PGM_P)(p)))
#define FE_PGM_STRNCMP(a, p, n) (strncmp_P((a), (PGM_P)(p), (n)))
#else
#define FE_PROGMEM
#define FE_PSTR(s)         (s)
#define FE_PGM_CHAR(p)     (*(const u8 *)(p))
#define FE_PGM_DWORD(p)    (*(const u32 *)(p))
#define FE_PGM_PTR(p)      (p)
#define FE_PGM_CMD_NAME(cmds, c) ((cmds)[c].name)
#define FE_PGM_EQ(a, s)    (strcmp((a), (s)) == 0)
#define FE_PGM_STRCMP(a, p) (strcmp((a), (p)))
#define FE_PGM_STRNCMP(a, p, n) (strncmp((a), (p), (n)))
#endif

// ============================================================
// 格式化输出（fe.c 统一使用）
// ============================================================
// 与 snprintf 语义一致：至多 size 字节、始终 NUL 结尾。
int fe_snprintf(char *buf, u16 size, const char *fmt, ...);
// flash 版本：fmt 在 flash，%S 参数也在 flash（AVR）；宿主回退普通 vsnprintf。
int fe_snprintf_P(char *buf, u16 size, const char *fmt_p, ...);

// ============================================================
// 串口（UART）
// ============================================================
typedef void (*fe_port_uart_rx_cb_t)(u8 byte, void *user);

// 初始化串口：port 编号（0=UART0/默认），baud 波特率，rx 回调（可为 NULL）
void fe_port_uart_init(u8 port, u32 baud, fe_port_uart_rx_cb_t rx_cb, void *user);
// 发送 len 字节，返回实际发送字节数
u16 fe_port_uart_write(u8 port, const u8 *data, u16 len);
// 发送一条 flash 字符串（AVR PROGMEM；宿主普通字符串）
void fe_port_uart_write_pgm(u8 port, const char *data_p);
// 是否有数据可读
u8  fe_port_uart_available(u8 port);
// 读一个字节（无数据返回 -1）
int fe_port_uart_read(u8 port);
// 关闭串口
void fe_port_uart_close(u8 port);

// ============================================================
// EEPROM（ATmega328P 内置 1KB 硬件 EEPROM）
// ============================================================
// 读字符串：addr 起始地址，out 输出缓冲，outlen 缓冲长度。成功返回 TRUE
u8 fe_port_eeprom_get_str(u16 addr, char *out, u16 outlen);
// 写字符串：addr 起始地址。成功返回 TRUE
u8 fe_port_eeprom_set_str(u16 addr, const char *value);
// 读 u32：成功返回 TRUE
u8 fe_port_eeprom_get_u32(u16 addr, u32 *out);
// 写 u32：成功返回 TRUE
u8 fe_port_eeprom_set_u32(u16 addr, u32 value);

// ============================================================
// 系统时间（epoch 秒，u32 到 2106 年够用）
// ============================================================
u32 fe_port_time_now(void);
void fe_port_time_set(u32 epoch);
// 启动 1ms 节拍（main 启动时调用一次）
void fe_port_timer0_init(void);

// ============================================================
// 随机数
// ============================================================
void fe_port_random_fill(u8 *buf, u16 len);

// ============================================================
// GPIO——GpioAbility 需要（Arduino 引脚号 0-19：D0-D13 + A0-A5）
// ============================================================
// 设置引脚模式：mode ∈ "input" / "output" / "input_pullup"。成功返回 0
int fe_port_gpio_set_mode(u8 pin, const char *mode);
// 输出电平：level 0/1。成功返回 0
int fe_port_gpio_write(u8 pin, u8 level);
// 读取电平，返回 0/1；失败返回 -1
int fe_port_gpio_read(u8 pin);

// ============================================================
// 芯片信息——ChipData 需要
// ============================================================
// 生成芯片信息 JSON 到 out
void fe_port_chip_info(char *out, u16 outlen);

// ============================================================
// 延时 / 喂狗（毫秒）
// ============================================================
void fe_port_delay_ms(u32 ms);

#endif // FE_PORT_H
