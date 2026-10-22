// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_base.c — BaseAbility 实现（C51/8051 版）
// list_data_names / list_ability_names
#include "fe_ability.h"
#include "fe_port.h"
#include <string.h>

fe_output_t ability_base_dispatch(void *inst, const char *act, const char *args) {
    (void)inst; (void)args;

    if (FE_PGM_STRCMP(act, FE_PSTR("list_data_names")) == 0) {
        char out[96];
        u16 n = (u16)fe_snprintf_P(out, sizeof(out), FE_PSTR("{\"names\":["));
        fe_list_data_names(fe_global_atom(), out + n, (u16)(sizeof(out) - n));
        n += (u16)strlen(out + n);
        fe_snprintf_P(out + n, (u16)(sizeof(out) - n), FE_PSTR("]}"));
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("list_ability_names")) == 0) {
        char out[96];
        u16 n = (u16)fe_snprintf_P(out, sizeof(out), FE_PSTR("{\"names\":["));
        fe_list_ability_names(fe_global_atom(), out + n, (u16)(sizeof(out) - n));
        n += (u16)strlen(out + n);
        fe_snprintf_P(out + n, (u16)(sizeof(out) - n), FE_PSTR("]}"));
        return fe_ok(act, out);
    }
    return fe_err_p(act, FE_PSTR("unsupported command"));
}
