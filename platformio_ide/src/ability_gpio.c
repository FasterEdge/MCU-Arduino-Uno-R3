// ability_gpio.c — GpioAbility 实现（Arduino Uno R3 / ATmega328P 版，MCU 专有）
// MCU 专有能力：Arduino 引脚 GPIO 控制。
//   mode <pin>,<input|output|input_pullup>  设置引脚模式
//   write <pin>,<0|1>                       输出电平
//   read <pin>                              读电平
//   info                                    说明
// pin 为 Arduino 引脚号 0-19（D0-D13 + A0-A5=14-19）。
#include "fe_ability.h"
#include "fe_port.h"
#include <string.h>
#include <stdlib.h>

fe_output_t ability_gpio_dispatch(void *inst, const char *act, const char *args) {
    char tmp[32];
    char *end;
    u8 pin;

    (void)inst;
    fe_snprintf(tmp, sizeof(tmp), "%s", args ? args : "");

    if (strcmp(act, "mode") == 0) {
        char *comma = strchr(tmp, ',');
        if (!comma) return fe_err(act, "bad format, expect pin,mode");
        *comma = 0;
        pin = (u8)strtoul(tmp, &end, 0);
        if (pin > 19) return fe_err(act, "pin must be 0-19");
        if (fe_port_gpio_set_mode(pin, comma + 1) != 0)
            return fe_err(act, "mode must be input/output/input_pullup");
        {
            char out[32];
            fe_snprintf(out, sizeof(out), "{\"pin\":%u,\"mode\":\"%s\"}",
                        (unsigned)pin, comma + 1);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "write") == 0) {
        char *comma = strchr(tmp, ',');
        if (!comma) return fe_err(act, "bad format, expect pin,value");
        *comma = 0;
        pin = (u8)strtoul(tmp, &end, 0);
        if (pin > 19) return fe_err(act, "pin must be 0-19");
        {
            u32 val = strtoul(comma + 1, &end, 0);
            if (val > 1) return fe_err(act, "value must be 0/1");
            if (fe_port_gpio_write(pin, (u8)val) != 0) return fe_err(act, "write failed");
            char out[32];
            fe_snprintf(out, sizeof(out), "{\"pin\":%u,\"value\":%u}",
                        (unsigned)pin, (unsigned)val);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "read") == 0) {
        int v;
        pin = (u8)strtoul(tmp, &end, 0);
        if (pin > 19) return fe_err(act, "pin must be 0-19");
        v = fe_port_gpio_read(pin);
        if (v < 0) return fe_err(act, "read failed");
        {
            char out[32];
            fe_snprintf(out, sizeof(out), "{\"pin\":%u,\"value\":%d}",
                        (unsigned)pin, v);
            return fe_ok(act, out);
        }
    }
    if (strcmp(act, "info") == 0) {
        return fe_ok(act,
            "{\"ability\":\"GpioAbility\",\"desc\":\"Arduino 引脚 GPIO\","
            "\"pins\":\"0-19\",\"width\":1}");
    }
    return fe_err(act, "unsupported command");
}