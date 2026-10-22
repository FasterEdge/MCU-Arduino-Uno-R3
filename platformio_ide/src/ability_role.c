// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_role.c — RoleAbility 实现（C51/8051 版）
// describe / set_role / get_role
#include "fe_ability.h"
#include "fe_port.h"
#include <string.h>

static role_ability_t s_role;

fe_output_t ability_role_dispatch(void *inst, const char *act, const char *args) {
    role_ability_t *self = &s_role;
    (void)inst;

    if (FE_PGM_STRCMP(act, FE_PSTR("describe")) == 0) {
        char out[80];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("{\"name\":\"RoleAbility\",\"role\":\"%s\"}"), self->role);
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("set_role")) == 0) {
        if (!args || args[0] == 0)
            return fe_err_p(act, FE_PSTR("missing role"));
        if (FE_PGM_STRCMP(args, FE_PSTR("edge")) != 0 &&
            FE_PGM_STRCMP(args, FE_PSTR("cloud")) != 0 &&
            FE_PGM_STRCMP(args, FE_PSTR("standalone")) != 0)
            return fe_err_p(act, FE_PSTR("invalid role"));
        fe_snprintf_P(self->role, sizeof(self->role), FE_PSTR("%s"), args);
        char out[48];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("role=%s"), self->role);
        return fe_ok(act, out);
    }
    if (FE_PGM_STRCMP(act, FE_PSTR("get_role")) == 0) {
        char out[48];
        fe_snprintf_P(out, sizeof(out), FE_PSTR("role=%s"), self->role);
        return fe_ok(act, out);
    }
    return fe_err_p(act, FE_PSTR("unsupported command"));
}
