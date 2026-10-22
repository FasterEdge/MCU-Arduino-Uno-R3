// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_role.cpp — RoleAbility 实现（Arduino 版）
// describe / set_role / get_role
#include "fe_ability.h"

namespace fe {

CommandOutput roleAbilityDispatch(void *inst, const char *act, const String &args) {
    RoleAbility *self = static_cast<RoleAbility *>(inst);

    if (FE_EQ_P(act, "describe")) {
        return CommandOutput{String(act),
            String(F("{\"name\":\"RoleAbility\",\"role\":\"")) + self->role + F("\"}"), String()};
    }
    if (FE_EQ_P(act, "set_role")) {
        if (args.length() == 0)
            return CommandOutput{String(act), String(), String(F("missing role arg"))};
        // 合法角色：edge / cloud / standalone
        String r = args;
        if (strcmp_P(r.c_str(), PSTR("edge")) != 0 && strcmp_P(r.c_str(), PSTR("cloud")) != 0 &&
            strcmp_P(r.c_str(), PSTR("standalone")) != 0)
            return CommandOutput{String(act), String(), String(F("invalid role: ")) + r};
        self->role = r;
        return CommandOutput{String(act), String(F("role=")) + r, String()};
    }
    if (FE_EQ_P(act, "get_role")) {
        return CommandOutput{String(act), String(F("role=")) + self->role, String()};
    }
    return CommandOutput{String(act), String(), String(F("unsupported command: ")) + act};
}

} // namespace fe
