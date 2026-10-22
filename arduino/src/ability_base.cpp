// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_base.cpp — BaseAbility 实现（Arduino 版）
// list_data_names / list_ability_names
#include "fe_ability.h"

namespace fe {

static String makeJsonNameList(const String &names) {
    String out = F("[");
    bool first = true;
    int start = 0;
    while (start <= names.length()) {
        int end = names.indexOf(',', start);
        String item = end >= 0 ? names.substring(start, end) : names.substring(start);
        if (item.length() > 0) {
            if (!first) out += F(",");
            out += F("\"");
            out += item;
            out += F("\"");
            first = false;
        }
        if (end < 0) break;
        start = end + 1;
    }
    out += F("]");
    return out;
}

CommandOutput baseAbilityDispatch(void *inst, const char *act, const String &args) {
    (void)inst; (void)args;
    BaseAbility *self = static_cast<BaseAbility *>(inst);

    if (FE_EQ_P(act, "list_data_names")) {
        String out = makeJsonNameList(globalAtom().listDataNames());
        return CommandOutput{String(act), out, String()};
    }
    if (FE_EQ_P(act, "list_ability_names")) {
        String out = makeJsonNameList(globalAtom().listAbilityNames());
        return CommandOutput{String(act), out, String()};
    }
    return CommandOutput{String(act), String(), String(F("unsupported command: ")) + act};
}

} // namespace fe
