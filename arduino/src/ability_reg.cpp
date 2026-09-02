// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_reg.cpp — RegAbility 实现（Arduino Uno R3 / ATmega328P 版，MCU 专有）
// MCU 专有能力：内存映射寄存器读写（8 位，AVR I/O 空间 0x00-0xFF）。
#include "fe_ability.h"
namespace fe {
static uint8_t hexVal(char c) {
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A' + 10);
    return 0xFF;
}
static bool parseHex(const String &s, uint32_t &val) {
    val = 0; uint8_t d;
    size_t i = 0;
    if (s.startsWith("0x") || s.startsWith("0X")) i = 2;
    for (; i < s.length(); i++) {
        d = hexVal(s.charAt(i));
        if (d == 0xFF) return false;
        val = (val << 4) | d;
    }
    return true;
}
CommandOutput regAbilityDispatch(void *inst, const char *act, const String &args) {
    (void)inst;
    if (strcmp(act, "read") == 0) {
        uint32_t addr;
        if (!parseHex(args, addr) || addr > 0xFF)
            return CommandOutput{String(act), String(), String("bad address (0x00-0xFF)")};
        uint8_t v = *(volatile uint8_t *)(uintptr_t)addr;
        char buf[48];
        snprintf(buf, sizeof(buf), "{\"addr\":\"0x%02X\",\"value\":%u,\"hex\":\"0x%02X\"}",
                 (unsigned)addr, (unsigned)v, (unsigned)v);
        return CommandOutput{String(act), String(buf), String()};
    }
    if (strcmp(act, "write") == 0) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String("bad format, expect addr,val")};
        uint32_t addr, val;
        if (!parseHex(args.substring(0, comma), addr) || addr > 0xFF)
            return CommandOutput{String(act), String(), String("bad address (0x00-0xFF)")};
        if (!parseHex(args.substring(comma + 1), val) || val > 0xFF)
            return CommandOutput{String(act), String(), String("bad value (0x00-0xFF)")};
        *(volatile uint8_t *)(uintptr_t)addr = (uint8_t)val;
        char buf[48];
        snprintf(buf, sizeof(buf), "{\"addr\":\"0x%02X\",\"value\":%u,\"hex\":\"0x%02X\"}",
                 (unsigned)addr, (unsigned)val, (unsigned)val);
        return CommandOutput{String(act), String(buf), String()};
    }
    if (strcmp(act, "bit_set") == 0) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String("bad format, expect addr,bit")};
        uint32_t addr, bit;
        if (!parseHex(args.substring(0, comma), addr) || addr > 0xFF)
            return CommandOutput{String(act), String(), String("bad address (0x00-0xFF)")};
        bit = args.substring(comma + 1).toInt();
        if (bit > 7) return CommandOutput{String(act), String(), String("bit must be 0..7")};
        uint8_t v = *(volatile uint8_t *)(uintptr_t)addr;
        v |= (uint8_t)(1u << bit);
        *(volatile uint8_t *)(uintptr_t)addr = v;
        char buf[48];
        snprintf(buf, sizeof(buf), "{\"addr\":\"0x%02X\",\"bit\":%lu,\"value\":%u}",
                 (unsigned)addr, (unsigned long)bit, (unsigned)v);
        return CommandOutput{String(act), String(buf), String()};
    }
    if (strcmp(act, "bit_clear") == 0) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String("bad format, expect addr,bit")};
        uint32_t addr, bit;
        if (!parseHex(args.substring(0, comma), addr) || addr > 0xFF)
            return CommandOutput{String(act), String(), String("bad address (0x00-0xFF)")};
        bit = args.substring(comma + 1).toInt();
        if (bit > 7) return CommandOutput{String(act), String(), String("bit must be 0..7")};
        uint8_t v = *(volatile uint8_t *)(uintptr_t)addr;
        v &= (uint8_t)~(1u << bit);
        *(volatile uint8_t *)(uintptr_t)addr = v;
        char buf[48];
        snprintf(buf, sizeof(buf), "{\"addr\":\"0x%02X\",\"bit\":%lu,\"value\":%u}",
                 (unsigned)addr, (unsigned long)bit, (unsigned)v);
        return CommandOutput{String(act), String(buf), String()};
    }
    if (strcmp(act, "info") == 0) {
        return CommandOutput{String(act),
            String("{\"ability\":\"RegAbility\",\"desc\":\"AVR 8 位寄存器\",\"addr\":\"0x00-0xFF\"}"), String()};
    }
    return CommandOutput{String(act), String(), String("unsupported command: ") + act};
}
} // namespace fe
