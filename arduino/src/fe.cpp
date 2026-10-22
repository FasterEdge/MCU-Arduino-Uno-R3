// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// fe.cpp — FasterEdge MCU 核心框架实现（Arduino 版）
#include "fe.h"

namespace fe {

// ============================================================
// Atom 实现
// ============================================================
String Atom::listDataNames() const {
    String names;
    for (uint8_t i = 0; i < dataCount_; i++) {
        if (i) names += F(",");
        names += String((const __FlashStringHelper *)dataList_[i].name);
    }
    return names;
}

String Atom::listAbilityNames() const {
    String names;
    for (uint8_t i = 0; i < abilityCount_; i++) {
        if (i) names += F(",");
        names += String((const __FlashStringHelper *)abilityList_[i].name);
    }
    return names;
}

CommandOutput Atom::execute(const char *target, const char *act, const String &args) {
    // 路由：data_xxx / ability_xxx
    bool isData = strncmp_P(target, PSTR("data_"), 5) == 0;
    const char *base = isData ? target + 5 : (strncmp_P(target, PSTR("ability_"), 8) == 0 ? target + 8 : target);

    if (isData) {
        for (uint8_t i = 0; i < dataCount_; i++) {
            const DataDef &d = dataList_[i];
            if (strcmp_P(base, (PGM_P)d.name) == 0) {
                for (size_t i = 0; i < d.cmd_count; i++) {
                    if (strcmp_P(act, (PGM_P)d.cmds[i].name) == 0)
                        return d.dispatch(d.instance, act, args);
                }
                return CommandOutput{String(act), String(), String(F("unsupported command: ")) + act};
            }
        }
        return CommandOutput{String(act), String(), String(F("unknown data: ")) + base};
    }

    for (uint8_t i = 0; i < abilityCount_; i++) {
        const AbilityDef &a = abilityList_[i];
        if (strcmp_P(base, (PGM_P)a.name) == 0) {
            for (size_t i = 0; i < a.cmd_count; i++) {
                if (strcmp_P(act, (PGM_P)a.cmds[i].name) == 0)
                    return a.dispatch(a.instance, act, args);
            }
            return CommandOutput{String(act), String(), String(F("unsupported command: ")) + act};
        }
    }
    return CommandOutput{String(act), String(), String(F("unknown ability: ")) + base};
}

// ============================================================
// 全局单例
// ============================================================
Atom &globalAtom() {
    static Atom atom;
    return atom;
}

// ============================================================
// 工具
// ============================================================
String toJsonString(const char *key, const String &value) {
    return String(F("{\"")) + key + F("\":\"") + value + F("\"}");
}

String toJsonNumber(const char *key, long value) {
    return String(F("{\"")) + key + F("\":") + value + F("}");
}

String kvPair(const char *key, const String &value) {
    return String(key) + F("=") + value;
}

} // namespace fe
