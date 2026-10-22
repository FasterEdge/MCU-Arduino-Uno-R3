// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_serial.c — SerialAbility 实现（C51/8051 版）
// open / close / write / read / is_open / set_config / get_config / list_ports
#include "fe_ability.h"
#include "fe_port.h"
#include <stdlib.h>
#include <string.h>

static serial_ability_t s_serial = { FALSE, 115200, 0 };

fe_output_t ability_serial_dispatch(void *inst, const char *act, const char *args) {
    serial_ability_t *self = &s_serial;
    (void)inst;

    if (FE_PGM_STRCMP(act, FE_PSTR("list_ports")) == 0) {
        return fe_ok_p(act, FE_PSTR("{\"ports\":[0]}"));
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("set_config")) == 0) {
        // 参数：port,baud（逗号分隔，可省略）
        u8 port = self->port;
        u32 baud = self->baud;
        if (args && args[0]) {
            char buf[32];
            char *comma;
            fe_snprintf_P(buf, sizeof(buf), FE_PSTR("%s"), args);
            comma = strchr(buf, ',');
            port = (u8)atoi(buf);
            if (comma) baud = (u32)atoi(comma + 1);
        }
        self->port = port;
        self->baud = baud;
        char out[48];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("port=%u baud=%lu"), port, (unsigned long)baud);
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("get_config")) == 0) {
        char out[64];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("{\"open\":%S,\"baud\":%lu,\"port\":%u}"),
                    self->open ? FE_PSTR("true") : FE_PSTR("false"), (unsigned long)self->baud, self->port);
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("open")) == 0) {
        u8 port = self->port;
        if (args && args[0]) port = (u8)atoi(args);
        fe_port_uart_init(port, self->baud, NULL, NULL);
        self->port = port;
        self->open = TRUE;
        char out[32];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("port=%u opened"), port);
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("close")) == 0) {
        fe_port_uart_close(self->port);
        self->open = FALSE;
        return fe_ok_p(act, FE_PSTR("closed"));
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("is_open")) == 0) {
        char out[32];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("{\"open\":%S}"),
                      self->open ? FE_PSTR("true") : FE_PSTR("false"));
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("write")) == 0) {
        u16 n;
        char out[24];
        if (!self->open) return fe_err_p(act, FE_PSTR("port not open"));
        n = fe_port_uart_write(self->port, (const u8 *)(args ? args : ""),
                               args ? (u16)strlen(args) : 0);
        fe_snprintf_P(out, sizeof(out), FE_PSTR("bytes=%u"), (unsigned)n);
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("read")) == 0) {
        char hex[96];
        u16 n = 0;
        int b;
        if (!self->open) return fe_err_p(act, FE_PSTR("port not open"));
        while (fe_port_uart_available(self->port) && n + 2 < sizeof(hex)) {
            b = fe_port_uart_read(self->port);
            if (b < 0) break;
            fe_snprintf_P(hex + n, sizeof(hex) - n, FE_PSTR("%02X"), b & 0xff);
            n += 2;
        }
        if (n == 0) return fe_ok_p(act, FE_PSTR(""));
        return fe_ok(act, hex);
    }
    return fe_err_p(act, FE_PSTR("unsupported command"));
}
