// register.c — 注册全部 Data / Ability 到全局 Atom（C51/8051 版）
// 注意：C51 对 C99 复合字面量支持有限，模块全部用显式静态变量。
#include "fe.h"
#include "fe_ability.h"
#include "fe_data.h"
#include "fe_port.h"

// ============================================================
// Flash 字符串（ATmega328P 2KB SRAM 下必须存入 flash）
// ============================================================
static const char s_n_base_data[]    FE_PROGMEM = "BaseData";
static const char s_n_config_data[]  FE_PROGMEM = "ConfigData";
static const char s_n_chip_data[]    FE_PROGMEM = "ChipData";
static const char s_n_base_ability[] FE_PROGMEM = "BaseAbility";
static const char s_n_role_ability[] FE_PROGMEM = "RoleAbility";
static const char s_n_time_ability[] FE_PROGMEM = "TimeAbility";
static const char s_n_onekey_ability[] FE_PROGMEM = "OneKeyAbility";
static const char s_n_serial_ability[] FE_PROGMEM = "SerialAbility";
static const char s_n_modbus_ability[] FE_PROGMEM = "ModbusAbility";
static const char s_n_reg_ability[] FE_PROGMEM = "RegAbility";
static const char s_n_gpio_ability[] FE_PROGMEM = "GpioAbility";

static const char s_c_logo[]        FE_PROGMEM = "logo";
static const char s_c_info[]        FE_PROGMEM = "info";
static const char s_c_get[]         FE_PROGMEM = "get";
static const char s_c_set[]         FE_PROGMEM = "set";
static const char s_c_delete[]      FE_PROGMEM = "delete";
static const char s_c_list[]        FE_PROGMEM = "list";
static const char s_c_snapshot[]    FE_PROGMEM = "snapshot";
static const char s_c_list_data_names[]  FE_PROGMEM = "list_data_names";
static const char s_c_list_ability_names[] FE_PROGMEM = "list_ability_names";
static const char s_c_describe[]    FE_PROGMEM = "describe";
static const char s_c_set_role[]    FE_PROGMEM = "set_role";
static const char s_c_get_role[]    FE_PROGMEM = "get_role";
static const char s_c_sync_manual[] FE_PROGMEM = "sync_manual";
static const char s_c_sync_system[] FE_PROGMEM = "sync_system";
static const char s_c_get_time[]    FE_PROGMEM = "get_time";
static const char s_c_configure_run[] FE_PROGMEM = "configure_run";
static const char s_c_issue_token[] FE_PROGMEM = "issue_token";
static const char s_c_verify_token[] FE_PROGMEM = "verify_token";
static const char s_c_revoke_all[]  FE_PROGMEM = "revoke_all";
static const char s_c_list_tokens[] FE_PROGMEM = "list_tokens";
static const char s_c_status[]      FE_PROGMEM = "status";
static const char s_c_rotate[]      FE_PROGMEM = "rotate";
static const char s_c_open[]        FE_PROGMEM = "open";
static const char s_c_close[]       FE_PROGMEM = "close";
static const char s_c_write[]       FE_PROGMEM = "write";
static const char s_c_read[]        FE_PROGMEM = "read";
static const char s_c_is_open[]     FE_PROGMEM = "is_open";
static const char s_c_set_config[]  FE_PROGMEM = "set_config";
static const char s_c_get_config[]  FE_PROGMEM = "get_config";
static const char s_c_list_ports[]  FE_PROGMEM = "list_ports";
static const char s_c_set_unit_id[] FE_PROGMEM = "set_unit_id";
static const char s_c_get_unit_id[] FE_PROGMEM = "get_unit_id";
static const char s_c_read_holding[] FE_PROGMEM = "read_holding";
static const char s_c_read_input[]  FE_PROGMEM = "read_input";
static const char s_c_read_coils[]  FE_PROGMEM = "read_coils";
static const char s_c_read_discrete[] FE_PROGMEM = "read_discrete";
static const char s_c_write_holding[] FE_PROGMEM = "write_holding";
static const char s_c_write_coil[]  FE_PROGMEM = "write_coil";
static const char s_c_bit_set[]     FE_PROGMEM = "bit_set";
static const char s_c_bit_clear[]   FE_PROGMEM = "bit_clear";
static const char s_c_mode[]        FE_PROGMEM = "mode";

// ============================================================
// Data 命令表
// ============================================================
static const fe_cmd_t s_base_cmds[] FE_PROGMEM = {
    {s_c_logo},
    {s_c_info},
};
static const fe_cmd_t s_config_cmds[] FE_PROGMEM = {
    {s_c_get},
    {s_c_set},
    {s_c_delete},
    {s_c_list},
    {s_c_snapshot},
};
static const fe_cmd_t s_chip_cmds[] FE_PROGMEM = {
    {s_c_info},
};

// ============================================================
// Ability 命令表
// ============================================================
static const fe_cmd_t s_ability_base_cmds[] FE_PROGMEM = {
    {s_c_list_data_names},
    {s_c_list_ability_names},
};
static const fe_cmd_t s_role_cmds[] FE_PROGMEM = {
    {s_c_describe},
    {s_c_set_role},
    {s_c_get_role},
};
static const fe_cmd_t s_time_cmds[] FE_PROGMEM = {
    {s_c_sync_manual},
    {s_c_sync_system},
    {s_c_get_time},
    {s_c_configure_run},
};
static const fe_cmd_t s_onekey_cmds[] FE_PROGMEM = {
    {s_c_issue_token},
    {s_c_verify_token},
    {s_c_revoke_all},
    {s_c_list_tokens},
    {s_c_status},
    {s_c_rotate},
};
static const fe_cmd_t s_serial_cmds[] FE_PROGMEM = {
    {s_c_open},
    {s_c_close},
    {s_c_write},
    {s_c_read},
    {s_c_is_open},
    {s_c_set_config},
    {s_c_get_config},
    {s_c_list_ports},
};
static const fe_cmd_t s_modbus_cmds[] FE_PROGMEM = {
    {s_c_set_unit_id},
    {s_c_get_unit_id},
    {s_c_read_holding},
    {s_c_read_input},
    {s_c_read_coils},
    {s_c_read_discrete},
    {s_c_write_holding},
    {s_c_write_coil},
};
static const fe_cmd_t s_reg_cmds[] FE_PROGMEM = {
    {s_c_read},
    {s_c_write},
    {s_c_bit_set},
    {s_c_bit_clear},
    {s_c_info},
};
static const fe_cmd_t s_gpio_cmds[] FE_PROGMEM = {
    {s_c_mode},
    {s_c_write},
    {s_c_read},
    {s_c_info},
};

// ============================================================
// 模块定义（显式静态变量，C51 兼容）
// ============================================================
static const fe_module_t s_data_base FE_PROGMEM = {
    s_n_base_data, s_base_cmds, (u8)(sizeof(s_base_cmds) / sizeof(s_base_cmds[0])), data_base_dispatch
};
static const fe_module_t s_data_config FE_PROGMEM = {
    s_n_config_data, s_config_cmds, (u8)(sizeof(s_config_cmds) / sizeof(s_config_cmds[0])), data_config_dispatch
};
static const fe_module_t s_data_chip FE_PROGMEM = {
    s_n_chip_data, s_chip_cmds, (u8)(sizeof(s_chip_cmds) / sizeof(s_chip_cmds[0])), data_chip_dispatch
};

static const fe_module_t s_ability_base FE_PROGMEM = {
    s_n_base_ability, s_ability_base_cmds, (u8)(sizeof(s_ability_base_cmds) / sizeof(s_ability_base_cmds[0])), ability_base_dispatch
};
static const fe_module_t s_ability_role FE_PROGMEM = {
    s_n_role_ability, s_role_cmds, (u8)(sizeof(s_role_cmds) / sizeof(s_role_cmds[0])), ability_role_dispatch
};
static const fe_module_t s_ability_time FE_PROGMEM = {
    s_n_time_ability, s_time_cmds, (u8)(sizeof(s_time_cmds) / sizeof(s_time_cmds[0])), ability_time_dispatch
};
static const fe_module_t s_ability_onekey FE_PROGMEM = {
    s_n_onekey_ability, s_onekey_cmds, (u8)(sizeof(s_onekey_cmds) / sizeof(s_onekey_cmds[0])), ability_onekey_dispatch
};
static const fe_module_t s_ability_serial FE_PROGMEM = {
    s_n_serial_ability, s_serial_cmds, (u8)(sizeof(s_serial_cmds) / sizeof(s_serial_cmds[0])), ability_serial_dispatch
};
static const fe_module_t s_ability_modbus FE_PROGMEM = {
    s_n_modbus_ability, s_modbus_cmds, (u8)(sizeof(s_modbus_cmds) / sizeof(s_modbus_cmds[0])), ability_modbus_dispatch
};
static const fe_module_t s_ability_reg FE_PROGMEM = {
    s_n_reg_ability, s_reg_cmds, (u8)(sizeof(s_reg_cmds) / sizeof(s_reg_cmds[0])), ability_reg_dispatch
};
static const fe_module_t s_ability_gpio FE_PROGMEM = {
    s_n_gpio_ability, s_gpio_cmds, (u8)(sizeof(s_gpio_cmds) / sizeof(s_gpio_cmds[0])), ability_gpio_dispatch
};

// ============================================================
// 注册 Data
// ============================================================
void fe_register_all_data(fe_atom_t *atom) {
    fe_register_data(atom, &s_data_base);
    fe_register_data(atom, &s_data_config);
    fe_register_data(atom, &s_data_chip);
}

// ============================================================
// 注册 Ability
// ============================================================
void fe_register_all_abilities(fe_atom_t *atom) {
    fe_register_ability(atom, &s_ability_base);
    fe_register_ability(atom, &s_ability_role);
    fe_register_ability(atom, &s_ability_time);
    fe_register_ability(atom, &s_ability_onekey);
    fe_register_ability(atom, &s_ability_serial);
    fe_register_ability(atom, &s_ability_modbus);
    fe_register_ability(atom, &s_ability_reg);
    fe_register_ability(atom, &s_ability_gpio);
}

// ============================================================
// 初始化全部
// ============================================================
void fe_init_all(void) {
    fe_atom_t *atom = fe_global_atom();
    memset(atom, 0, sizeof(*atom));
    fe_register_all_data(atom);
    fe_register_all_abilities(atom);
}
