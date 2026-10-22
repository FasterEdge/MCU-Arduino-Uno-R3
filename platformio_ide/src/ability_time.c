// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_time.c — TimeAbility 实现（C51/8051 版）
// sync_manual / sync_system / get_time / configure_run
// C51 无网络，不含 sync_ntp/sync_net。时间源由 fe_port 抽象
// （RTC 芯片 / 定时器计数）。epoch 用 u32（到 2106 年够用）。
#include "fe_ability.h"
#include "fe_port.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static time_ability_t s_time;

fe_output_t ability_time_dispatch(void *inst, const char *act, const char *args) {
    time_ability_t *self = &s_time;
    (void)inst;

    if (FE_PGM_STRCMP(act, FE_PSTR("get_time")) == 0) {
        u32 now = fe_port_time_now();
        char out[48];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("{\"epoch\":%lu}"), (unsigned long)now);
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("sync_manual")) == 0) {
        if (!args || args[0] == 0)
            return fe_err_p(act, FE_PSTR("missing epoch"));
        if (args[0] == '-') return fe_err_p(act, FE_PSTR("invalid epoch"));
        char *endp = NULL;
        errno = 0;
        unsigned long raw = strtoul(args, &endp, 10);
        if (errno == ERANGE || *endp != 0)
            return fe_err_p(act, FE_PSTR("invalid epoch"));
        u32 ep = (u32)raw;
        if (ep == 0) return fe_err_p(act, FE_PSTR("invalid epoch"));
        fe_port_time_set(ep);
        char out[48];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("epoch=%lu"), (unsigned long)ep);
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("sync_system")) == 0) {
        u32 now = fe_port_time_now();
        char out[48];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("epoch=%lu"), (unsigned long)now);
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("configure_run")) == 0) {
        u32 interval;
        char out[80];
        if (!args || !args[0]) {
            fe_snprintf_P(out, sizeof(out),
                        FE_PSTR("{\"enabled\":%S,\"interval\":%lu,\"nextRun\":%lu}"),
                        self->run_enabled ? FE_PSTR("true") : FE_PSTR("false"),
                        (unsigned long)self->run_interval,
                        (unsigned long)self->next_run);
            return fe_ok(act, out);
        }
        interval = (u32)strtoul(args, NULL, 10);
        if (interval == 0) {
            self->run_enabled = FALSE;
            self->run_interval = 0;
            self->next_run = 0;
        } else {
            self->run_enabled = TRUE;
            self->run_interval = interval;
            u32 now = fe_port_time_now();
            self->next_run = (interval > 0xFFFFFFFFUL - now) ? 0xFFFFFFFFUL : now + interval;
        }
        fe_snprintf_P(out, sizeof(out),
                    FE_PSTR("{\"enabled\":%S,\"interval\":%lu,\"nextRun\":%lu}"),
                    self->run_enabled ? FE_PSTR("true") : FE_PSTR("false"),
                    (unsigned long)self->run_interval,
                    (unsigned long)self->next_run);
        return fe_ok(act, out);
    }
    return fe_err_p(act, FE_PSTR("unsupported command"));
}
