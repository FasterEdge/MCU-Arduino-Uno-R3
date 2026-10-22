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
    static const CommandEntry baseDataCmds[] = {{PSTR("logo"), baseDataDispatch},{PSTR("info"), baseDataDispatch}};
    static const CommandEntry configDataCmds[] = {
        {PSTR("get"), configDataDispatch},{PSTR("set"), configDataDispatch},{PSTR("delete"), configDataDispatch},
        {PSTR("list"), configDataDispatch},{PSTR("snapshot"), configDataDispatch}};
    static const CommandEntry chipDataCmds[] = {{PSTR("info"), chipDataDispatch}};
    atom.registerData({ PSTR("BaseData"),   PSTR("框架元信息"), baseDataCmds,   sizeof(baseDataCmds)/sizeof(baseDataCmds[0]),   &g_baseData,   baseDataDispatch });
    atom.registerData({ PSTR("ConfigData"), PSTR("KV 配置(EEPROM)"), configDataCmds, sizeof(configDataCmds)/sizeof(configDataCmds[0]), &g_configData, configDataDispatch });
    atom.registerData({ PSTR("ChipData"),   PSTR("芯片信息(MCU 专有)"), chipDataCmds, sizeof(chipDataCmds)/sizeof(chipDataCmds[0]), &g_chipData,   chipDataDispatch });
}
void registerAllAbilities(Atom &atom) {
    static const CommandEntry baseAbilityCmds[] = {{PSTR("list_data_names"), baseAbilityDispatch},{PSTR("list_ability_names"), baseAbilityDispatch}};
    static const CommandEntry roleAbilityCmds[] = {{PSTR("describe"), roleAbilityDispatch},{PSTR("set_role"), roleAbilityDispatch},{PSTR("get_role"), roleAbilityDispatch}};
    static const CommandEntry timeAbilityCmds[] = {{PSTR("sync_manual"), timeAbilityDispatch},{PSTR("sync_system"), timeAbilityDispatch},{PSTR("get_time"), timeAbilityDispatch},{PSTR("configure_run"), timeAbilityDispatch}};
    static const CommandEntry oneKeyAbilityCmds[] = {{PSTR("issue_token"), oneKeyAbilityDispatch},{PSTR("verify_token"), oneKeyAbilityDispatch},{PSTR("revoke_all"), oneKeyAbilityDispatch},{PSTR("list_tokens"), oneKeyAbilityDispatch},{PSTR("status"), oneKeyAbilityDispatch},{PSTR("rotate"), oneKeyAbilityDispatch}};
    static const CommandEntry serialAbilityCmds[] = {{PSTR("open"), serialAbilityDispatch},{PSTR("close"), serialAbilityDispatch},{PSTR("write"), serialAbilityDispatch},{PSTR("read"), serialAbilityDispatch},{PSTR("is_open"), serialAbilityDispatch},{PSTR("set_config"), serialAbilityDispatch},{PSTR("get_config"), serialAbilityDispatch},{PSTR("list_ports"), serialAbilityDispatch}};
    static const CommandEntry modbusAbilityCmds[] = {{PSTR("set_unit_id"), modbusAbilityDispatch},{PSTR("get_unit_id"), modbusAbilityDispatch},{PSTR("read_holding"), modbusAbilityDispatch},{PSTR("read_input"), modbusAbilityDispatch},{PSTR("read_coils"), modbusAbilityDispatch},{PSTR("read_discrete"), modbusAbilityDispatch},{PSTR("write_holding"), modbusAbilityDispatch},{PSTR("write_coil"), modbusAbilityDispatch}};
    static const CommandEntry regAbilityCmds[] = {{PSTR("read"), regAbilityDispatch},{PSTR("write"), regAbilityDispatch},{PSTR("bit_set"), regAbilityDispatch},{PSTR("bit_clear"), regAbilityDispatch},{PSTR("info"), regAbilityDispatch}};
    static const CommandEntry gpioAbilityCmds[] = {{PSTR("mode"), gpioAbilityDispatch},{PSTR("write"), gpioAbilityDispatch},{PSTR("read"), gpioAbilityDispatch},{PSTR("info"), gpioAbilityDispatch}};
    atom.registerAbility({ PSTR("BaseAbility"),   PSTR("基础"),   baseAbilityCmds,   sizeof(baseAbilityCmds)/sizeof(baseAbilityCmds[0]),   &g_baseAbility,   baseAbilityDispatch });
    atom.registerAbility({ PSTR("RoleAbility"),   PSTR("角色"),   roleAbilityCmds,   sizeof(roleAbilityCmds)/sizeof(roleAbilityCmds[0]),   &g_roleAbility,   roleAbilityDispatch });
    atom.registerAbility({ PSTR("TimeAbility"),   PSTR("时间"),   timeAbilityCmds,   sizeof(timeAbilityCmds)/sizeof(timeAbilityCmds[0]),   &g_timeAbility,   timeAbilityDispatch });
    atom.registerAbility({ PSTR("OneKeyAbility"), PSTR("一键令牌"), oneKeyAbilityCmds, sizeof(oneKeyAbilityCmds)/sizeof(oneKeyAbilityCmds[0]), &g_oneKeyAbility, oneKeyAbilityDispatch });
    atom.registerAbility({ PSTR("SerialAbility"), PSTR("串口"),   serialAbilityCmds, sizeof(serialAbilityCmds)/sizeof(serialAbilityCmds[0]), &g_serialAbility, serialAbilityDispatch });
    atom.registerAbility({ PSTR("ModbusAbility"), PSTR("Modbus"), modbusAbilityCmds, sizeof(modbusAbilityCmds)/sizeof(modbusAbilityCmds[0]), &g_modbusAbility, modbusAbilityDispatch });
    atom.registerAbility({ PSTR("RegAbility"),    PSTR("寄存器操作(专有)"), regAbilityCmds, sizeof(regAbilityCmds)/sizeof(regAbilityCmds[0]), &g_regAbility,   regAbilityDispatch });
    atom.registerAbility({ PSTR("GpioAbility"),   PSTR("引脚 GPIO(专有)"), gpioAbilityCmds, sizeof(gpioAbilityCmds)/sizeof(gpioAbilityCmds[0]), &g_gpioAbility, gpioAbilityDispatch });
    EEPROM.begin();     // AVR 库的 begin() 无参数；ESP32 才支持 begin(size)
    g_serialAbility.open = false; g_serialAbility.baud = 115200;
    g_modbusAbility.unitId = 1;
}
void initAll() {
    registerAllData(globalAtom());
    registerAllAbilities(globalAtom());
}
} // namespace fe
