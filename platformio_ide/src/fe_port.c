/* FasterEdge 开源项目
 * GitHub: https://github.com/FasterEdge
 * Gitee:  https://gitee.com/FasterEdge
 */
// fe_port.c — FasterEdge MCU 平台移植层实现（Arduino Uno R3 / ATmega328P 版）
// 真实实现：寄存器级操作（avr-libc）。
//   UART0  : USART0 寄存器（UBRR0/UCSR0A/B/C/UDR0）
//   EEPROM : avr/eeprom.h（内置 1KB 硬件 EEPROM）
//   GPIO   : DDRx/PORTx/PINx + Arduino 引脚映射表（D0-D13 + A0-A5=14-19）
//   time   : Timer0 CTC 1ms 中断计数（类似 Arduino millis）
#include "fe_port.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================
// 格式化输出（委托 vsnprintf；AVR libc 支持 %s/%d/%u/%x/%lu）
// ============================================================
int fe_snprintf(char *buf, u16 size, const char *fmt, ...) {
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    if (n < 0) { buf[0] = 0; return 0; }
    if ((u16)n >= size) buf[size - 1] = 0;
    return n;
}

// ============================================================
// 串口（USART0）
// ============================================================
static volatile u8 s_rx_buf[64];
static volatile u8 s_rx_head = 0, s_rx_tail = 0;

ISR(USART_RX_vect) {
    u8 b = UDR0;
    u8 next = (u8)((s_rx_head + 1) & 63);
    if (next != s_rx_tail) {
        s_rx_buf[s_rx_head] = b;
        s_rx_head = next;
    }
}

void fe_port_uart_init(u8 port, u32 baud, fe_port_uart_rx_cb_t rx_cb, void *user) {
    u16 ubrr;
    (void)port; (void)rx_cb; (void)user;
    // 波特率：UBRR = F_CPU/16/baud - 1（U2X 关闭）
    ubrr = (u16)((F_CPU / 16UL / baud) - 1);
    UBRR0H = (u8)(ubrr >> 8);
    UBRR0L = (u8)ubrr;
    UCSR0B = 0;                      // 先关闭
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);   // 8N1
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);   // 收发 + RX 中断
    sei();
}

u16 fe_port_uart_write(u8 port, const u8 *data, u16 len) {
    u16 i;
    (void)port;
    for (i = 0; i < len; i++) {
        while (!(UCSR0A & (1 << UDRE0))) ;   // 等待发送缓冲空
        UDR0 = data[i];
    }
    return len;
}

u8 fe_port_uart_available(u8 port) {
    (void)port;
    return s_rx_head != s_rx_tail;
}

int fe_port_uart_read(u8 port) {
    u8 b;
    (void)port;
    if (s_rx_head == s_rx_tail) return -1;
    b = s_rx_buf[s_rx_tail];
    s_rx_tail = (u8)((s_rx_tail + 1) & 63);
    return b;
}

void fe_port_uart_close(u8 port) {
    (void)port;
    UCSR0B = 0;   // 关闭收发
}

// ============================================================
// EEPROM（avr/eeprom.h）
// ============================================================
#define EEPROM_SIZE 1024   // ATmega328P 内置 1KB

u8 fe_port_eeprom_get_str(u16 addr, char *out, u16 outlen) {
    u16 i;
    if (addr + 1 > EEPROM_SIZE || outlen == 0) return FALSE;
    for (i = 0; i + 1 < outlen; i++) {
        u8 c = eeprom_read_byte((const u8 *)(addr + i));
        out[i] = (char)c;
        if (c == 0) break;
    }
    out[i] = 0;
    return TRUE;
}

u8 fe_port_eeprom_set_str(u16 addr, const char *value) {
    u16 i;
    u16 n = (u16)strlen(value) + 1;
    if (addr + n > EEPROM_SIZE) return FALSE;
    for (i = 0; i < n; i++)
        eeprom_write_byte((u8 *)(addr + i), (u8)value[i]);
    return TRUE;
}

u8 fe_port_eeprom_get_u32(u16 addr, u32 *out) {
    u8 b[4];
    u8 i;
    if (addr + 4 > EEPROM_SIZE) return FALSE;
    for (i = 0; i < 4; i++) b[i] = eeprom_read_byte((const u8 *)(addr + i));
    *out = (u32)b[0] | ((u32)b[1] << 8) | ((u32)b[2] << 16) | ((u32)b[3] << 24);
    return TRUE;
}

u8 fe_port_eeprom_set_u32(u16 addr, u32 value) {
    u8 i;
    if (addr + 4 > EEPROM_SIZE) return FALSE;
    for (i = 0; i < 4; i++)
        eeprom_write_byte((u8 *)(addr + i), (u8)(value >> (i * 8)));
    return TRUE;
}

// ============================================================
// 系统时间（Timer0 CTC 1ms tick + epoch 基数）
// ============================================================
volatile u32 s_tick_ms = 0;
static u32 s_epoch = 0;

ISR(TIMER0_COMPA_vect) {
    s_tick_ms++;
}

u32 fe_port_time_now(void) {
    return s_epoch + s_tick_ms / 1000UL;
}

void fe_port_time_set(u32 epoch) {
    s_epoch = epoch;
}

// 启动 1ms 节拍：F_CPU=16MHz, 预分频 64, CTC 匹配 249 -> 250 ticks = 1ms
void fe_port_timer0_init(void) {
    TCCR0A = (1 << WGM01);                 // CTC 模式
    TCCR0B = (1 << CS01) | (1 << CS00);    // 预分频 /64
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);                // 比较匹配中断
}

// ============================================================
// 随机数（LCG；可扩展 ADC 噪声熵源）
// ============================================================
static u32 s_rng = 0xFEEDBEEFUL;

void fe_port_random_fill(u8 *buf, u16 len) {
    u16 i;
    for (i = 0; i < len; i++) {
        s_rng = s_rng * 1664525UL + 1013904223UL;   // LCG
        buf[i] = (u8)(s_rng >> 24);
    }
}

// ============================================================
// GPIO（Arduino 引脚 0-19 -> 端口/位。D0-D7=PD, D8-D13=PB,
// A0-A5=14-19=PC）
// ============================================================
static int pin_port_bit(u8 pin, volatile uint8_t **ddr, volatile uint8_t **port,
                        volatile uint8_t **pin_reg, u8 *bit) {
    if (pin <= 7)      { *ddr = &DDRD; *port = &PORTD; *pin_reg = &PIND; *bit = pin; return 0; }
    else if (pin <= 13){ *ddr = &DDRB; *port = &PORTB; *pin_reg = &PINB; *bit = (u8)(pin - 8); return 0; }
    else if (pin <= 19){ *ddr = &DDRC; *port = &PORTC; *pin_reg = &PINC; *bit = (u8)(pin - 14); return 0; }
    return -1;
}

int fe_port_gpio_set_mode(u8 pin, const char *mode) {
    volatile uint8_t *ddr, *port;
    volatile uint8_t *pin_reg;
    u8 bit;
    if (pin_port_bit(pin, &ddr, &port, &pin_reg, &bit) != 0) return -1;
    if (strcmp(mode, "output") == 0) {
        *ddr |= (u8)(1 << bit);
    } else if (strcmp(mode, "input") == 0) {
        *ddr &= (u8)~(1 << bit);
        *port &= (u8)~(1 << bit);       // 关闭上拉
    } else if (strcmp(mode, "input_pullup") == 0) {
        *ddr &= (u8)~(1 << bit);
        *port |= (u8)(1 << bit);        // 使能内部上拉
    } else {
        return -1;
    }
    return 0;
}

int fe_port_gpio_write(u8 pin, u8 level) {
    volatile uint8_t *ddr, *port;
    volatile uint8_t *pin_reg;
    u8 bit;
    if (pin_port_bit(pin, &ddr, &port, &pin_reg, &bit) != 0) return -1;
    if (level) *port |= (u8)(1 << bit);
    else       *port &= (u8)~(1 << bit);
    return 0;
}

int fe_port_gpio_read(u8 pin) {
    volatile uint8_t *ddr, *port;
    volatile uint8_t *pin_reg;
    u8 bit;
    if (pin_port_bit(pin, &ddr, &port, &pin_reg, &bit) != 0) return -1;
    return (*pin_reg >> bit) & 1;
}

// ============================================================
// 芯片信息
// ============================================================
void fe_port_chip_info(char *out, u16 outlen) {
    fe_snprintf(out, outlen,
                "{\"chip\":\"ATmega328P\",\"arch\":\"AVR\","
                "\"ramBytes\":2048,\"flashBytes\":32768,\"eepromBytes\":1024,"
                "\"freqMHz\":%lu}",
                (unsigned long)(F_CPU / 1000000UL));
}

// ============================================================
// 延时（软件循环；如需精确可改用 Timer0）
// ============================================================
void fe_port_delay_ms(u32 ms) {
    volatile u32 i;
    for (; ms > 0; ms--)
        for (i = 0; i < 16000UL / 4UL; i++) ;   // 近似 1ms @16MHz
}