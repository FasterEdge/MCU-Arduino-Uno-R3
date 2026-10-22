// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_gpio.cpp — GpioAbility 实现（Arduino Uno R3 / ATmega328P 版，MCU 专有）
// MCU 专有能力：Arduino 引脚 GPIO 控制（pin 0-19: D0-D13 + A0-A5=14-19）。
#include "fe_ability.h"
#include <Arduino.h>
namespace fe {
CommandOutput gpioAbilityDispatch(void *inst, const char *act, const String &args) {
    (void)inst;
    if (FE_EQ_P(act, "mode")) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String(F("bad format, expect pin,mode"))};
        int pin = args.substring(0, comma).toInt();
        String mode = args.substring(comma + 1);
        if (pin < 0 || pin > 19) return CommandOutput{String(act), String(), String(F("pin must be 0-19"))};
        if (strcmp_P(mode.c_str(), PSTR("input")) == 0) pinMode(pin, INPUT);
        else if (strcmp_P(mode.c_str(), PSTR("output")) == 0) pinMode(pin, OUTPUT);
        else if (strcmp_P(mode.c_str(), PSTR("input_pullup")) == 0) pinMode(pin, INPUT_PULLUP);
        else return CommandOutput{String(act), String(), String(F("mode must be input/output/input_pullup"))};
        return CommandOutput{String(act), String(F("{\"pin\":")) + pin + F(",\"mode\":\"") + mode + F("\"}"), String()};
    }
    if (FE_EQ_P(act, "write")) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String(F("bad format, expect pin,value"))};
        int pin = args.substring(0, comma).toInt();
        int val = args.substring(comma + 1).toInt();
        if (pin < 0 || pin > 19) return CommandOutput{String(act), String(), String(F("pin must be 0-19"))};
        if (val != 0 && val != 1) return CommandOutput{String(act), String(), String(F("value must be 0/1"))};
        digitalWrite(pin, val);
        return CommandOutput{String(act), String(F("{\"pin\":")) + pin + F(",\"value\":") + val + F("}"), String()};
    }
    if (FE_EQ_P(act, "read")) {
        int pin = args.toInt();
        if (pin < 0 || pin > 19) return CommandOutput{String(act), String(), String(F("pin must be 0-19"))};
        int v = digitalRead(pin);
        return CommandOutput{String(act), String(F("{\"pin\":")) + pin + F(",\"value\":") + v + F("}"), String()};
    }
    if (FE_EQ_P(act, "info")) {
        return CommandOutput{String(act),
            String(F("{\"ability\":\"GpioAbility\",\"desc\":\"Arduino 引脚 GPIO\",\"pins\":\"0-19\"}")), String()};
    }
    return CommandOutput{String(act), String(), String(F("unsupported command: ")) + act};
}
} // namespace fe
