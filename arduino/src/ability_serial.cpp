// ability_serial.cpp — SerialAbility 实现（Arduino Uno R3 版）
// 单 UART（Serial over USB/UART0）。
#include "fe_ability.h"
#include <Arduino.h>
namespace fe {
CommandOutput serialAbilityDispatch(void *inst, const char *act, const String &args) {
    SerialAbility *self = static_cast<SerialAbility *>(inst);
    if (strcmp(act, "list_ports") == 0) {
        return CommandOutput{String(act), String("{\"ports\":[0]}"), String()};
    }
    if (strcmp(act, "set_config") == 0) {
        if (args.length()) {
            int comma = args.indexOf(',');
            self->baud = (comma > 0) ? args.substring(comma + 1).toInt() : 115200;
        }
        return CommandOutput{String(act), String("port=0 baud=") + self->baud, String()};
    }
    if (strcmp(act, "get_config") == 0) {
        String out = String("{\"open\":") + (self->open ? "true" : "false")
                     + ",\"baud\":" + self->baud + ",\"port\":0}";
        return CommandOutput{String(act), out, String()};
    }
    if (strcmp(act, "open") == 0) {
        Serial.begin(self->baud);
        self->open = true;
        return CommandOutput{String(act), String("port=0 opened"), String()};
    }
    if (strcmp(act, "close") == 0) {
        Serial.end(); self->open = false;
        return CommandOutput{String(act), String("closed"), String()};
    }
    if (strcmp(act, "is_open") == 0) {
        return CommandOutput{String(act), String("{\"open\":") + (self->open ? "true" : "false") + "}", String()};
    }
    if (strcmp(act, "write") == 0) {
        if (!self->open) return CommandOutput{String(act), String(), String("port not open")};
        Serial.print(args);
        return CommandOutput{String(act), String("bytes=") + args.length(), String()};
    }
    if (strcmp(act, "read") == 0) {
        if (!self->open) return CommandOutput{String(act), String(), String("port not open")};
        String hex;
        while (Serial.available()) {
            int b = Serial.read();
            if (b < 0) break;
            char buf[4]; snprintf(buf, sizeof(buf), "%02X", b & 0xFF);
            hex += buf;
        }
        return CommandOutput{String(act), hex, String()};
    }
    return CommandOutput{String(act), String(), String("unsupported command: ") + act};
}
} // namespace fe
