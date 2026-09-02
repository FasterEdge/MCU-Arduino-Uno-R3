// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// register.cpp — 注册全部 Data / Ability 到全局 Atom（Arduino Uno R3 版）
#include "fe.h"
#include "fe_ability.h"
#include "fe_data.h"
#include <EEPROM.h>
namespace fe {
static BaseAbility       g_baseAbility;
static RoleAbility       g_roleAbility;
static TimeAbility       g_timeAbility;
static OneKeyAbility     g_oneKeyAbility;
static SerialAbility     g_serialAbility;
static ModbusAbility     g_modbusAbility;
static RegAbility        g_regAbility;
static GpioAbility       g_gpioAbility;
static BaseData    g_baseData;
static ConfigData  g_configData;
static ChipData    g_chipData;
void registerAllData(Atom &atom) {
    static const CommandEntry baseDataCmds[] = {{"logo", baseDataDispatch},{"info", baseDataDispatch}};
    static const CommandEntry configDataCmds[] = {
        {"get", configDataDispatch},{"set", configDataDispatch},{"delete", configDataDispatch},
        {"list", configDataDispatch},{"snapshot", configDataDispatch}};
    static const CommandEntry chipDataCmds[] = {{"info", chipDataDispatch}};
    atom.registerData({ "BaseData",   "框架元信息", baseDataCmds,   sizeof(baseDataCmds)/sizeof(baseDataCmds[0]),   &g_baseData,   baseDataDispatch });
    atom.registerData({ "ConfigData", "KV 配置(EEPROM)", configDataCmds, sizeof(configDataCmds)/sizeof(configDataCmds[0]), &g_configData, configDataDispatch });
    atom.registerData({ "ChipData",   "芯片信息(MCU 专有)", chipDataCmds, sizeof(chipDataCmds)/sizeof(chipDataCmds[0]), &g_chipData,   chipDataDispatch });
}
void registerAllAbilities(Atom &atom) {
    static const CommandEntry baseAbilityCmds[] = {{"list_data_names", baseAbilityDispatch},{"list_ability_names", baseAbilityDispatch}};
    static const CommandEntry roleAbilityCmds[] = {{"describe", roleAbilityDispatch},{"set_role", roleAbilityDispatch},{"get_role", roleAbilityDispatch}};
    static const CommandEntry timeAbilityCmds[] = {{"sync_manual", timeAbilityDispatch},{"sync_system", timeAbilityDispatch},{"get_time", timeAbilityDispatch},{"configure_run", timeAbilityDispatch}};
    static const CommandEntry oneKeyAbilityCmds[] = {{"issue_token", oneKeyAbilityDispatch},{"verify_token", oneKeyAbilityDispatch},{"revoke_all", oneKeyAbilityDispatch},{"list_tokens", oneKeyAbilityDispatch},{"status", oneKeyAbilityDispatch},{"rotate", oneKeyAbilityDispatch}};
    static const CommandEntry serialAbilityCmds[] = {{"open", serialAbilityDispatch},{"close", serialAbilityDispatch},{"write", serialAbilityDispatch},{"read", serialAbilityDispatch},{"is_open", serialAbilityDispatch},{"set_config", serialAbilityDispatch},{"get_config", serialAbilityDispatch},{"list_ports", serialAbilityDispatch}};
    static const CommandEntry modbusAbilityCmds[] = {{"set_unit_id", modbusAbilityDispatch},{"get_unit_id", modbusAbilityDispatch},{"read_holding", modbusAbilityDispatch},{"read_input", modbusAbilityDispatch},{"read_coils", modbusAbilityDispatch},{"read_discrete", modbusAbilityDispatch},{"write_holding", modbusAbilityDispatch},{"write_coil", modbusAbilityDispatch}};
    static const CommandEntry regAbilityCmds[] = {{"read", regAbilityDispatch},{"write", regAbilityDispatch},{"bit_set", regAbilityDispatch},{"bit_clear", regAbilityDispatch},{"info", regAbilityDispatch}};
    static const CommandEntry gpioAbilityCmds[] = {{"mode", gpioAbilityDispatch},{"write", gpioAbilityDispatch},{"read", gpioAbilityDispatch},{"info", gpioAbilityDispatch}};
    atom.registerAbility({ "BaseAbility",   "基础",   baseAbilityCmds,   sizeof(baseAbilityCmds)/sizeof(baseAbilityCmds[0]),   &g_baseAbility,   baseAbilityDispatch });
    atom.registerAbility({ "RoleAbility",   "角色",   roleAbilityCmds,   sizeof(roleAbilityCmds)/sizeof(roleAbilityCmds[0]),   &g_roleAbility,   roleAbilityDispatch });
    atom.registerAbility({ "TimeAbility",   "时间",   timeAbilityCmds,   sizeof(timeAbilityCmds)/sizeof(timeAbilityCmds[0]),   &g_timeAbility,   timeAbilityDispatch });
    atom.registerAbility({ "OneKeyAbility", "一键令牌", oneKeyAbilityCmds, sizeof(oneKeyAbilityCmds)/sizeof(oneKeyAbilityCmds[0]), &g_oneKeyAbility, oneKeyAbilityDispatch });
    atom.registerAbility({ "SerialAbility", "串口",   serialAbilityCmds, sizeof(serialAbilityCmds)/sizeof(serialAbilityCmds[0]), &g_serialAbility, serialAbilityDispatch });
    atom.registerAbility({ "ModbusAbility", "Modbus", modbusAbilityCmds, sizeof(modbusAbilityCmds)/sizeof(modbusAbilityCmds[0]), &g_modbusAbility, modbusAbilityDispatch });
    atom.registerAbility({ "RegAbility",    "寄存器操作(专有)", regAbilityCmds, sizeof(regAbilityCmds)/sizeof(regAbilityCmds[0]), &g_regAbility,   regAbilityDispatch });
    atom.registerAbility({ "GpioAbility",   "引脚 GPIO(专有)", gpioAbilityCmds, sizeof(gpioAbilityCmds)/sizeof(gpioAbilityCmds[0]), &g_gpioAbility, gpioAbilityDispatch });
    EEPROM.begin(1024);
    g_serialAbility.open = false; g_serialAbility.baud = 115200;
    g_modbusAbility.unitId = 1;
    g_modbusAbility.holdingRegs.assign(32, 0);
    g_modbusAbility.inputRegs.assign(32, 0);
    g_modbusAbility.coils.assign(32, false);
    g_modbusAbility.discreteInputs.assign(32, false);
}
void initAll() {
    registerAllData(globalAtom());
    registerAllAbilities(globalAtom());
}
} // namespace fe
