// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// fe.h — FasterEdge MCU 核心框架（Arduino 版）
// 模仿 FasterEdge 主仓库的 Atom / Ability / Data / Command 模型
// 平台：ESP32 (Arduino core / PlatformIO)
#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifdef __AVR__
#define FE_EQ_P(a, s)        (strcmp_P((a), PSTR(s)) == 0)
#define FE_STRN_EQ_P(a, s)   (strncmp_P((a), PSTR(s), sizeof(s) - 1) == 0)
#else
#define FE_EQ_P(a, s)        (strcmp((a), (s)) == 0)
#define FE_STRN_EQ_P(a, s)   (strncmp((a), (s), sizeof(s) - 1) == 0)
#endif

namespace fe {

enum { FE_MAX_DATA = 16, FE_MAX_ABILITY = 16 };

// ============================================================
// 基础类型
// ============================================================

// 命令输出：与 FasterEdge 的 CommandOutput 对应
struct CommandOutput {
    String name;      // 命令名
    String value;     // 返回的 JSON / 文本
    String err;       // 错误信息（空表示成功）
    bool ok() const { return err.length() == 0; }
};

// 命令执行回调：void* 为各 Ability/Data 实例
typedef CommandOutput (*CommandHandler)(void *inst, const char *act, const String &args);

// 命令表项
struct CommandEntry {
    const char *name;   // 命令名，如 "sync_net"
    CommandHandler handler;
};

// ============================================================
// Data 定义（框架元数据）
// ============================================================
struct DataDef {
    const char *name;       // 如 "BaseData"
    const char *desc;       // 描述
    const CommandEntry *cmds;   // 命令表
    size_t cmd_count;
    void *instance;         // 实现实例
    CommandHandler dispatch;
};

// ============================================================
// Ability 定义
// ============================================================
struct AbilityDef {
    const char *name;       // 如 "BaseAbility"
    const char *desc;       // 描述
    const CommandEntry *cmds;   // 命令表
    size_t cmd_count;
    void *instance;
    CommandHandler dispatch;
};

// ============================================================
// Atom：运行单元（对应 FasterEdge 的 Atom）
// ============================================================
class Atom {
public:
    Atom() = default;

    // 注册
    void registerData(const DataDef &d) {
        if (dataCount_ < FE_MAX_DATA) dataList_[dataCount_++] = d;
    }
    void registerAbility(const AbilityDef &a) {
        if (abilityCount_ < FE_MAX_ABILITY) abilityList_[abilityCount_++] = a;
    }

    // 查询
    String listDataNames() const;
    String listAbilityNames() const;

    // 执行命令：data_xxx / ability_xxx 前缀路由
    CommandOutput execute(const char *target, const char *act, const String &args);

private:
    DataDef    dataList_[FE_MAX_DATA];
    AbilityDef abilityList_[FE_MAX_ABILITY];
    uint8_t    dataCount_ = 0;
    uint8_t    abilityCount_ = 0;
};

// ============================================================
// 全局 Atom（单例）
// ============================================================
Atom &globalAtom();

// 初始化：注册全部 Data / Ability
void initAll();

// ============================================================
// 工具
// ============================================================
String toJsonString(const char *key, const String &value);
String toJsonNumber(const char *key, long value);
String kvPair(const char *key, const String &value);  // "key=value" 兼容形式

} // namespace fe
