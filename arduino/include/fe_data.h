// fe_data.h — FasterEdge MCU Data 模块声明（Arduino Uno R3 版）
// Base / Config（Keyring 合并入 OneKeyAbility，NetMap 因无网络省略）
#pragma once
#include "fe.h"
namespace fe {

struct BaseData {
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput baseDataDispatch(void *inst, const char *act, const String &args);

struct ConfigData {
    String ns;
    ConfigData() : ns("fe") {}
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput configDataDispatch(void *inst, const char *act, const String &args);

struct ChipData {
    CommandOutput dispatch(const char *act, const String &args);
};
extern CommandOutput chipDataDispatch(void *inst, const char *act, const String &args);

void registerAllData(Atom &atom);
} // namespace fe
