// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_time.cpp — TimeAbility 实现（Arduino Uno R3 版）
// sync_manual / sync_system / get_time / configure_run（无网络，无 NTP）
#include "fe_ability.h"
namespace fe {
static long parseEpoch(const String &s) {
    if (s.length() == 0) return 0;
    bool numeric = true;
    for (size_t i = 0; i < s.length(); i++)
        if (!isdigit(s[i])) { numeric = false; break; }
    if (numeric) return s.toInt();
    return 0;
}
CommandOutput timeAbilityDispatch(void *inst, const char *act, const String &args) {
    TimeAbility *self = static_cast<TimeAbility *>(inst);
    if (strcmp(act, "get_time") == 0) {
        long epoch = self->manualEpoch + (long)(millis() / 1000);
        return CommandOutput{String(act), String("epoch=") + epoch, String()};
    }
    if (strcmp(act, "sync_manual") == 0) {
        long ep = parseEpoch(args);
        if (ep <= 0) return CommandOutput{String(act), String(), String("invalid epoch")};
        self->manualEpoch = ep - (long)(millis() / 1000);
        return CommandOutput{String(act), String("epoch=") + ep, String()};
    }
    if (strcmp(act, "sync_system") == 0) {
        long epoch = self->manualEpoch + (long)(millis() / 1000);
        return CommandOutput{String(act), String("epoch=") + epoch, String()};
    }
    if (strcmp(act, "configure_run") == 0) {
        return CommandOutput{String(act), String("configured"), String()};
    }
    return CommandOutput{String(act), String(), String("unsupported command: ") + act};
}
} // namespace fe
