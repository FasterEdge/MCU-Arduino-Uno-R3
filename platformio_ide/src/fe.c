// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// fe.c — FasterEdge MCU 核心框架实现（C51/8051 版）
// 注意：输出格式化统一使用 fe_snprintf（在 fe_port.h 声明、fe_port.c 提供），
// 因为 Keil C51 标准库无 snprintf，且对 %ld/%lu 支持有限。
#include "fe.h"
#include "fe_port.h"
#include <string.h>

// ============================================================
// 注册
// ============================================================
void fe_register_data(fe_atom_t *atom, const fe_module_t *mod) {
    if (atom->data_count >= FE_MAX_MODULES) return;
#ifdef __AVR__
    memcpy_P(&atom->data[atom->data_count], mod, sizeof(fe_module_t));
#else
    memcpy(&atom->data[atom->data_count], mod, sizeof(fe_module_t));
#endif
    atom->data_count++;
}

void fe_register_ability(fe_atom_t *atom, const fe_module_t *mod) {
    if (atom->ability_count >= FE_MAX_MODULES) return;
#ifdef __AVR__
    memcpy_P(&atom->ability[atom->ability_count], mod, sizeof(fe_module_t));
#else
    memcpy(&atom->ability[atom->ability_count], mod, sizeof(fe_module_t));
#endif
    atom->ability_count++;
}

// ============================================================
// 名称列表
// ============================================================
void fe_list_data_names(const fe_atom_t *atom, char *out, u16 outlen) {
    u16 n = 0;
    u8 i;
    out[0] = 0;
    for (i = 0; i < atom->data_count && n < outlen; i++) {
        int w = fe_snprintf_P(out + n, outlen - n, FE_PSTR("%S%S"),
                              i ? FE_PSTR(",") : FE_PSTR(""), atom->data[i].name);
        if (w < 0) break;
        n += (u16)w;
    }
}

void fe_list_ability_names(const fe_atom_t *atom, char *out, u16 outlen) {
    u16 n = 0;
    u8 i;
    out[0] = 0;
    for (i = 0; i < atom->ability_count && n < outlen; i++) {
        int w = fe_snprintf_P(out + n, outlen - n, FE_PSTR("%S%S"),
                              i ? FE_PSTR(",") : FE_PSTR(""), atom->ability[i].name);
        if (w < 0) break;
        n += (u16)w;
    }
}

// ============================================================
// 命令路由
// ============================================================
fe_output_t fe_execute(fe_atom_t *atom, const char *target, const char *act, const char *args) {
    u8 isData = FALSE;
    const char *base = target;
    if (FE_PGM_STRNCMP(target, FE_PSTR("data_"), 5) == 0) { isData = TRUE; base = target + 5; }
    else if (FE_PGM_STRNCMP(target, FE_PSTR("ability_"), 8) == 0) base = target + 8;

    u8 count = isData ? atom->data_count : atom->ability_count;
    const fe_module_t *mods = isData ? atom->data : atom->ability;
    u8 m, c;

    for (m = 0; m < count; m++) {
        if (FE_PGM_STRCMP(mods[m].name, base) == 0) {
            for (c = 0; c < mods[m].cmd_count; c++) {
                const char *cmd_name = FE_PGM_CMD_NAME(mods[m].cmds, c);
                if (FE_PGM_STRCMP(cmd_name, act) == 0) {
                    return mods[m].dispatch(NULL, act, args);
                }
            }
            return fe_err_p(act, FE_PSTR("unsupported command"));
        }
    }
    return fe_err_p(act, isData ? FE_PSTR("unknown data") : FE_PSTR("unknown ability"));
}

// ============================================================
// 输出工具
// ============================================================
fe_output_t fe_ok(const char *name, const char *value) {
    fe_output_t o;
    memset(&o, 0, sizeof(o));
    if (name) fe_snprintf_P(o.name, sizeof(o.name), FE_PSTR("%s"), name);
    if (value) fe_snprintf_P(o.value, sizeof(o.value), FE_PSTR("%s"), value);
    o.ok = TRUE;
    return o;
}

fe_output_t fe_err(const char *name, const char *err) {
    fe_output_t o;
    memset(&o, 0, sizeof(o));
    if (name) fe_snprintf_P(o.name, sizeof(o.name), FE_PSTR("%s"), name);
    if (err) fe_snprintf_P(o.err, sizeof(o.err), FE_PSTR("%s"), err);
    else fe_snprintf_P(o.err, sizeof(o.err), FE_PSTR("%S"), FE_PSTR("error"));
    o.ok = FALSE;
    return o;
}

fe_output_t fe_ok_p(const char *name, const char *value_p) {
    fe_output_t o;
    memset(&o, 0, sizeof(o));
    if (name) fe_snprintf_P(o.name, sizeof(o.name), FE_PSTR("%s"), name);
#ifdef __AVR__
    fe_snprintf_P(o.value, sizeof(o.value), FE_PSTR("%S"),
                  value_p ? value_p : FE_PSTR(""));
#else
    if (value_p) fe_snprintf_P(o.value, sizeof(o.value), FE_PSTR("%s"), value_p);
#endif
    o.ok = TRUE;
    return o;
}

fe_output_t fe_err_p(const char *name, const char *err_p) {
    fe_output_t o;
    memset(&o, 0, sizeof(o));
    if (name) fe_snprintf_P(o.name, sizeof(o.name), FE_PSTR("%s"), name);
#ifdef __AVR__
    fe_snprintf_P(o.err, sizeof(o.err), FE_PSTR("%S"),
                  err_p ? err_p : FE_PSTR("error"));
#else
    if (err_p) fe_snprintf_P(o.err, sizeof(o.err), FE_PSTR("%s"), err_p);
    else fe_snprintf_P(o.err, sizeof(o.err), FE_PSTR("%S"), FE_PSTR("error"));
#endif
    o.ok = FALSE;
    return o;
}

// ============================================================
// 全局 Atom（在 register.c 中填充）
// ============================================================
static fe_atom_t g_atom;

fe_atom_t *fe_global_atom(void) { return &g_atom; }

void fe_init_all(void);  // 由 register.c 实现
