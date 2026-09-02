// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_gpio.cpp — GpioAbility 实现（Arduino Uno R3 / ATmega328P 版，MCU 专有）
// MCU 专有能力：Arduino 引脚 GPIO 控制（pin 0-19: D0-D13 + A0-A5=14-19）。
#include "fe_ability.h"
#include <Arduino.h>
namespace fe {
CommandOutput gpioAbilityDispatch(void *inst, const char *act, const String &args) {
    (void)inst;
    if (strcmp(act, "mode") == 0) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String("bad format, expect pin,mode")};
        int pin = args.substring(0, comma).toInt();
        String mode = args.substring(comma + 1);
        if (pin < 0 || pin > 19) return CommandOutput{String(act), String(), String("pin must be 0-19")};
        if (mode == "input") pinMode(pin, INPUT);
        else if (mode == "output") pinMode(pin, OUTPUT);
        else if (mode == "input_pullup") pinMode(pin, INPUT_PULLUP);
        else return CommandOutput{String(act), String(), String("mode must be input/output/input_pullup")};
        return CommandOutput{String(act), String("{\"pin\":") + pin + ",\"mode\":\"" + mode + "\"}", String()};
    }
    if (strcmp(act, "write") == 0) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String("bad format, expect pin,value")};
        int pin = args.substring(0, comma).toInt();
        int val = args.substring(comma + 1).toInt();
        if (pin < 0 || pin > 19) return CommandOutput{String(act), String(), String("pin must be 0-19")};
        if (val != 0 && val != 1) return CommandOutput{String(act), String(), String("value must be 0/1")};
        digitalWrite(pin, val);
        return CommandOutput{String(act), String("{\"pin\":") + pin + ",\"value\":" + val + "}", String()};
    }
    if (strcmp(act, "read") == 0) {
        int pin = args.toInt();
        if (pin < 0 || pin > 19) return CommandOutput{String(act), String(), String("pin must be 0-19")};
        int v = digitalRead(pin);
        return CommandOutput{String(act), String("{\"pin\":") + pin + ",\"value\":" + v + "}", String()};
    }
    if (strcmp(act, "info") == 0) {
        return CommandOutput{String(act),
            String("{\"ability\":\"GpioAbility\",\"desc\":\"Arduino 引脚 GPIO\",\"pins\":\"0-19\"}"), String()};
    }
    return CommandOutput{String(act), String(), String("unsupported command: ") + act};
}
} // namespace fe
