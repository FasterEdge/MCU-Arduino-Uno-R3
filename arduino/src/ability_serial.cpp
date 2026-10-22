// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_serial.cpp — SerialAbility 实现（Arduino Uno R3 版）
// 单 UART（Serial over USB/UART0）。
#include "fe_ability.h"
#include <Arduino.h>
namespace fe {
CommandOutput serialAbilityDispatch(void *inst, const char *act, const String &args) {
    SerialAbility *self = static_cast<SerialAbility *>(inst);
    if (FE_EQ_P(act, "list_ports")) {
        return CommandOutput{String(act), String(F("{\"ports\":[0]}")), String()};
    }
    if (FE_EQ_P(act, "set_config")) {
        if (args.length()) {
            int comma = args.indexOf(',');
            self->baud = (comma > 0) ? args.substring(comma + 1).toInt() : 115200;
        }
        return CommandOutput{String(act), String(F("port=0 baud=")) + self->baud, String()};
    }
    if (FE_EQ_P(act, "get_config")) {
        String out = F("{\"open\":");
        out += self->open ? F("true") : F("false");
        out += F(",\"baud\":");
        out += self->baud;
        out += F(",\"port\":0}");
        return CommandOutput{String(act), out, String()};
    }
    if (FE_EQ_P(act, "open")) {
        Serial.begin(self->baud);
        self->open = true;
        return CommandOutput{String(act), String(F("port=0 opened")), String()};
    }
    if (FE_EQ_P(act, "close")) {
        Serial.end(); self->open = false;
        return CommandOutput{String(act), String(F("closed")), String()};
    }
    if (FE_EQ_P(act, "is_open")) {
        return CommandOutput{String(act), String(F("{\"open\":")) + (self->open ? F("true") : F("false")) + F("}"), String()};
    }
    if (FE_EQ_P(act, "write")) {
        if (!self->open) return CommandOutput{String(act), String(), String(F("port not open"))};
        Serial.print(args);
        return CommandOutput{String(act), String(F("bytes=")) + args.length(), String()};
    }
    if (FE_EQ_P(act, "read")) {
        if (!self->open) return CommandOutput{String(act), String(), String(F("port not open"))};
        String hex;
        while (Serial.available()) {
            int b = Serial.read();
            if (b < 0) break;
            char buf[4]; snprintf_P(buf, sizeof(buf), PSTR("%02X"), b & 0xFF);
            hex += buf;
        }
        return CommandOutput{String(act), hex, String()};
    }
    return CommandOutput{String(act), String(), String(F("unsupported command: ")) + act};
}
} // namespace fe
