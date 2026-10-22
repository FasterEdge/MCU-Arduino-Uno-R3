// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// main.cpp — FasterEdge MCU Arduino 入口（Arduino Uno R3）
// 串口命令解释器：输入 "data_xxx act args" / "ability_xxx act args"
#include "fe.h"
#include "fe_ability.h"
#include "fe_data.h"

using namespace fe;

String inputBuffer;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println(F("FasterEdge-MCU (ATmega328P / Arduino)"));
    Serial.println(F("input: <data|ability>_<Name> <act> [args]  |  'help'"));
    initAll();
}

void printHelp() {
    Serial.println(F("Usage: <data|ability>_<Name> <act> [args]"));
    auto &atom = globalAtom();
    String names = atom.listAbilityNames();
    Serial.print(F("abilities: "));
    int start = 0;
    while (start <= names.length()) {
        int end = names.indexOf(',', start);
        String n = end >= 0 ? names.substring(start, end) : names.substring(start);
        if (n.length() > 0) { Serial.print(n); Serial.print(' '); }
        if (end < 0) break;
        start = end + 1;
    }
    Serial.println();
    String dnames = atom.listDataNames();
    Serial.print(F("data: "));
    start = 0;
    while (start <= dnames.length()) {
        int end = dnames.indexOf(',', start);
        String n = end >= 0 ? dnames.substring(start, end) : dnames.substring(start);
        if (n.length() > 0) { Serial.print(n); Serial.print(' '); }
        if (end < 0) break;
        start = end + 1;
    }
    Serial.println();
    Serial.println(F("examples:"));
    Serial.println(F("  ability_BaseAbility list_ability_names"));
    Serial.println(F("  ability_RoleAbility set_role edge"));
    Serial.println(F("  ability_TimeAbility sync_manual 1700000000"));
    Serial.println(F("  ability_OneKeyAbility issue_token sensor01"));
    Serial.println(F("  ability_SerialAbility open 0"));
    Serial.println(F("  ability_ModbusAbility write_holding 0,42"));
    Serial.println(F("  data_ConfigData set wifi.ssid=MyNet"));
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            inputBuffer.trim();
            if (inputBuffer.length() == 0) { inputBuffer = ""; continue; }
            if (strcmp_P(inputBuffer.c_str(), PSTR("help")) == 0) { printHelp(); inputBuffer = ""; continue; }

            int sp1 = inputBuffer.indexOf(' ');
            if (sp1 <= 0) {
                Serial.println(F("bad command"));
                inputBuffer = "";
                continue;
            }
            String target = inputBuffer.substring(0, sp1);
            String rest = inputBuffer.substring(sp1 + 1);
            int sp2 = rest.indexOf(' ');
            String act, args;
            if (sp2 > 0) { act = rest.substring(0, sp2); args = rest.substring(sp2 + 1); }
            else         { act = rest; }

            CommandOutput out = globalAtom().execute(target.c_str(), act.c_str(), args);
            if (out.ok()) {
                Serial.print(F("OK "));
                Serial.print(out.name);
                if (out.value.length()) { Serial.print(F(" -> ")); Serial.print(out.value); }
                Serial.println();
            } else {
                Serial.print(F("ERR "));
                Serial.print(out.name);
                Serial.print(F(": "));
                Serial.println(out.err);
            }
            inputBuffer = "";
        } else if (c == '\r') {
            // ignore
        } else {
            if (inputBuffer.length() < 96) inputBuffer += c;  // 上限防无界 realloc 耗尽堆
        }
    }
    delay(10);
}
