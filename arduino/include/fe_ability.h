// fe_ability.h — FasterEdge MCU Ability 模块声明（Arduino Uno R3 版）
// 无网络合理子集：Base / Role / Time / OneKey / Serial / Modbus + Reg / Gpio
#pragma once
#include "fe.h"
namespace fe {

struct BaseAbility {
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput baseAbilityDispatch(void *inst, const char *act, const String &args);

struct RoleAbility {
    String role;
    RoleAbility() : role("edge") {}
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput roleAbilityDispatch(void *inst, const char *act, const String &args);

struct TimeAbility {
    long manualEpoch;
    TimeAbility() : manualEpoch(0) {}
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput timeAbilityDispatch(void *inst, const char *act, const String &args);

struct OneKeyAbility {
    String secret;
    size_t tokenSeq;
    OneKeyAbility() : tokenSeq(0) {}
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput oneKeyAbilityDispatch(void *inst, const char *act, const String &args);

struct SerialAbility {
    bool open;
    long baud;
    SerialAbility() : open(false), baud(115200) {}
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput serialAbilityDispatch(void *inst, const char *act, const String &args);

struct ModbusAbility {
    uint8_t unitId;
    std::vector<uint16_t> holdingRegs;
    std::vector<uint16_t> inputRegs;
    std::vector<bool>     coils;
    std::vector<bool>     discreteInputs;
    ModbusAbility();
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput modbusAbilityDispatch(void *inst, const char *act, const String &args);

struct RegAbility {
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput regAbilityDispatch(void *inst, const char *act, const String &args);

struct GpioAbility {
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput gpioAbilityDispatch(void *inst, const char *act, const String &args);

void registerAllAbilities(Atom &atom);
} // namespace fe
