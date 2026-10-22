// ─────────────────────────────────────────────────────────────
// FasterEdge 开源项目
// Github: https://github.com/FasterEdge
// Gitee:  https://gitee.com/FasterEdge
// ─────────────────────────────────────────────────────────────
// data_base.c — BaseData 实现（C51/8051 版）
// logo / info
#include "fe_data.h"
#include "fe_port.h"
#include <string.h>

static const char LOGO[] FE_PROGMEM =
    "  __ _        _\n"
    " / _| |_ _  _(_)__ _\n"
    "|  _|  _| || | / _` |\n"
    "|_|  \\__|\\_, |_\\__,_|\n"
    "        |__/\n";

fe_output_t data_base_dispatch(void *inst, const char *act, const char *args) {
    (void)inst; (void)args;
    if (FE_PGM_STRCMP(act, FE_PSTR("logo")) == 0) {
        return fe_ok_p(act, LOGO);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("info")) == 0) {
        return fe_ok_p(act, FE_PSTR(
            "{\"name\":\"BaseData\",\"firmware\":\"FasterEdge-MCU 1.0.20261022\","
            "\"chip\":\"ATmega328P\",\"sdk\":\"bare-metal AVR\"}"));
    }
    return fe_err_p(act, FE_PSTR("unsupported command"));
}
