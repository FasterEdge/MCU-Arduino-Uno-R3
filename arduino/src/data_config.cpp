// data_config.cpp — ConfigData 实现（Arduino Uno R3 版）
// 扁平点号路径 KV 配置（EEPROM 持久化）：get / set / delete / list / snapshot
#include "fe_data.h"
#include <EEPROM.h>
namespace fe {
#define CFG_SLOTS 16
#define CFG_KEY_LEN 16
#define CFG_VAL_LEN 32
#define CFG_ENTRY (CFG_KEY_LEN + CFG_VAL_LEN)
static String normalizeKey(const String &k) {
    String s = k;
    s.replace('.', '_'); s.replace('/', '_');
    if (s.length() > 15) s = s.substring(0, 15);
    return s;
}
static int findSlot(const String &key) {
    for (int i = 0; i < CFG_SLOTS; i++) {
        char k[CFG_KEY_LEN];
        for (int j = 0; j < CFG_KEY_LEN; j++) k[j] = (char)EEPROM.read(i * CFG_ENTRY + j);
        if (k[0] != 0 && (uint8_t)k[0] != 0xFF && String(k) == key) return i;
    }
    return -1;
}
static String readValue(int slot) {
    char v[CFG_VAL_LEN];
    int base = slot * CFG_ENTRY + CFG_KEY_LEN;
    for (int j = 0; j < CFG_VAL_LEN; j++) { v[j] = (char)EEPROM.read(base + j); if (v[j] == 0) break; }
    v[CFG_VAL_LEN - 1] = 0;
    return String(v);
}
static void writeValue(int slot, const String &value) {
    int base = slot * CFG_ENTRY + CFG_KEY_LEN;
    for (size_t j = 0; j < (size_t)CFG_VAL_LEN; j++) {
        EEPROM.write(base + j, (uint8_t)(j < value.length() ? value[j] : 0));
    }
}
CommandOutput configDataDispatch(void *inst, const char *act, const String &args) {
    (void)inst;
    if (strcmp(act, "get") == 0) {
        if (args.length() == 0) return CommandOutput{String(act), String(), String("missing key")};
        String key = normalizeKey(args);
        int slot = findSlot(key);
        if (slot < 0) return CommandOutput{String(act), String("{}"), String()};
        return CommandOutput{String(act), String("{\"") + key + "\":\"" + readValue(slot) + "\"}", String()};
    }
    if (strcmp(act, "set") == 0) {
        int eq = args.indexOf('=');
        if (eq <= 0) return CommandOutput{String(act), String(), String("bad format, expect key=value")};
        String key = normalizeKey(args.substring(0, eq));
        String value = args.substring(eq + 1);
        int slot = findSlot(key);
        if (slot < 0) {
            for (int i = 0; i < CFG_SLOTS; i++) {
                char c = (char)EEPROM.read(i * CFG_ENTRY);
                if (c == 0 || (uint8_t)c == 0xFF) { slot = i; break; }
            }
            if (slot < 0) return CommandOutput{String(act), String(), String("config full")};
            for (size_t j = 0; j < (size_t)CFG_KEY_LEN; j++)
                EEPROM.write(slot * CFG_ENTRY + j, (uint8_t)(j < key.length() ? key[j] : 0));
        }
        writeValue(slot, value);
        return CommandOutput{String(act), String("saved"), String()};
    }
    if (strcmp(act, "delete") == 0) {
        if (args.length() == 0) return CommandOutput{String(act), String(), String("missing key")};
        String key = normalizeKey(args);
        int slot = findSlot(key);
        if (slot >= 0) for (int j = 0; j < CFG_KEY_LEN; j++) EEPROM.write(slot * CFG_ENTRY + j, 0);
        return CommandOutput{String(act), String("deleted"), String()};
    }
    if (strcmp(act, "list") == 0) {
        String list;
        for (int i = 0; i < CFG_SLOTS; i++) {
            char k[CFG_KEY_LEN];
            for (int j = 0; j < CFG_KEY_LEN; j++) k[j] = (char)EEPROM.read(i * CFG_ENTRY + j);
            if (k[0] != 0 && (uint8_t)k[0] != 0xFF) {
                if (list.length()) list += ",";
                list += String("\"") + String(k).substring(0, strnlen(k, CFG_KEY_LEN)) + "\"";
            }
        }
        return CommandOutput{String(act), String("{\"keys\":[") + list + "]}", String()};
    }
    if (strcmp(act, "snapshot") == 0) {
        return CommandOutput{String(act), String("{\"snapshot\":{}}"), String()};
    }
    return CommandOutput{String(act), String(), String("unsupported command: ") + act};
}
} // namespace fe
