// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_reg.c — RegAbility 实现（Arduino Uno R3 / ATmega328P 版，MCU 专有）
// MCU 专有能力：内存映射寄存器读写（8 位，AVR I/O 空间 0x00-0xFF）。
//   read <addr>            读字节
//   write <addr>,<value>   写字节
//   bit_set <addr>,<bit>   置位
//   bit_clear <addr>,<bit> 清位
//   info                   说明
// 注意：请谨慎使用，误写寄存器可能导致系统异常。
#include "fe_ability.h"
#include "fe_port.h"
#include <string.h>
#include <stdlib.h>

// 寄存器访问宏：默认直接解引用内存映射地址；
// 单元测试可用 -DFE_REG8 覆盖为内存数组，避免宿主机段错误。
#ifndef FE_REG8
#define FE_REG8(a) (*(volatile u8 *)(unsigned long)(a))
#endif

static u8 hex_val(char c) {
    if (c >= '0' && c <= '9') return (u8)(c - '0');
    if (c >= 'a' && c <= 'f') return (u8)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (u8)(c - 'A' + 10);
    return 0xFF;
}

// 解析十六进制（支持 0x 前缀）；失败返回 0，*ok=FALSE
static u32 parse_hex(const char *s, u8 *ok) {
    u32 v = 0;
    u8 d;
    u8 n = 0;
    *ok = FALSE;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
    if (*s == 0) return 0;
    while (*s) {
        d = hex_val(*s);
        if (d == 0xFF) return 0;
        v = (v << 4) | d;
        s++;
        if (++n > 8) return 0;   // 超过 32 位地址上限
    }
    *ok = TRUE;
    return v;
}

fe_output_t ability_reg_dispatch(void *inst, const char *act, const char *args) {
    char tmp[40];
    char *comma;
    u32 addr;
    u8 ok;

    (void)inst;
    fe_snprintf(tmp, sizeof(tmp), "%s", args ? args : "");

    if (strcmp(act, "read") == 0) {
        addr = parse_hex(tmp, &ok);
        if (!ok || addr > 0xFF) return fe_err(act, "bad register address (0x00-0xFF)");
        {
            u8 v = FE_REG8(addr);
            char out[48];
            fe_snprintf(out, sizeof(out), "{\"addr\":\"0x%02X\",\"value\":%u,\"hex\":\"0x%02X\"}",
                        (unsigned)addr, (unsigned)v, (unsigned)v);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "write") == 0) {
        comma = strchr(tmp, ',');
        if (!comma) return fe_err(act, "bad format, expect addr,value");
        *comma = 0;
        addr = parse_hex(tmp, &ok);
        if (!ok || addr > 0xFF) return fe_err(act, "bad register address (0x00-0xFF)");
        {
            u32 val = parse_hex(comma + 1, &ok);
            if (!ok || val > 0xFF) return fe_err(act, "bad value (0x00-0xFF)");
            FE_REG8(addr) = (u8)val;
            char out[48];
            fe_snprintf(out, sizeof(out), "{\"addr\":\"0x%02X\",\"value\":%u,\"hex\":\"0x%02X\"}",
                        (unsigned)addr, (unsigned)val, (unsigned)val);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "bit_set") == 0) {
        comma = strchr(tmp, ',');
        if (!comma) return fe_err(act, "bad format, expect addr,bit");
        *comma = 0;
        addr = parse_hex(tmp, &ok);
        if (!ok || addr > 0xFF) return fe_err(act, "bad register address (0x00-0xFF)");
        {
            u32 bit = strtoul(comma + 1, NULL, 0);
            u8 v;
            if (bit > 7) return fe_err(act, "bit must be 0..7");
            v = FE_REG8(addr);
            v |= (u8)(1u << bit);
            FE_REG8(addr) = v;
            char out[48];
            fe_snprintf(out, sizeof(out), "{\"addr\":\"0x%02X\",\"bit\":%lu,\"value\":%u}",
                        (unsigned)addr, (unsigned long)bit, (unsigned)v);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "bit_clear") == 0) {
        comma = strchr(tmp, ',');
        if (!comma) return fe_err(act, "bad format, expect addr,bit");
        *comma = 0;
        addr = parse_hex(tmp, &ok);
        if (!ok || addr > 0xFF) return fe_err(act, "bad register address (0x00-0xFF)");
        {
            u32 bit = strtoul(comma + 1, NULL, 0);
            u8 v;
            if (bit > 7) return fe_err(act, "bit must be 0..7");
            v = FE_REG8(addr);
            v &= (u8)~(1u << bit);
            FE_REG8(addr) = v;
            char out[48];
            fe_snprintf(out, sizeof(out), "{\"addr\":\"0x%02X\",\"bit\":%lu,\"value\":%u}",
                        (unsigned)addr, (unsigned long)bit, (unsigned)v);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "info") == 0) {
        return fe_ok(act,
            "{\"ability\":\"RegAbility\",\"desc\":\"AVR 内存映射寄存器读写\","
            "\"addr\":\"0x00-0xFF\",\"width\":8}");
    }
    return fe_err(act, "unsupported command");
}
