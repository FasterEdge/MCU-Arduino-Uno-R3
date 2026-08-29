// data_chip.cpp — ChipData 实现（Arduino Uno R3 / ATmega328P 版，MCU 专有）
#include "fe_data.h"
namespace fe {
CommandOutput chipDataDispatch(void *inst, const char *act, const String &args) {
    (void)inst; (void)args;
    if (strcmp(act, "info") == 0) {
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "{\"chip\":\"ATmega328P\",\"arch\":\"AVR\",\"ramBytes\":2048,"
                 "\"flashBytes\":32768,\"eepromBytes\":1024,\"freqMHz\":%lu}",
                 (unsigned long)(F_CPU / 1000000UL));
        return CommandOutput{String(act), String(buf), String()};
    }
    return CommandOutput{String(act), String(), String("unsupported command: ") + act};
}
} // namespace fe
