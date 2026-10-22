// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_modbus.cpp — ModbusAbility 实现（Arduino 版）
// set_unit_id / read_holding / read_input / read_coils / read_discrete /
// write_holding / write_coil
// 本能力将 ESP32 作为 Modbus RTU 从站（寄存器表存于 RAM），
// 底层帧收发通过 Serial（RS485 透传需外接 MAX485）。
#include "fe_ability.h"

namespace fe {

ModbusAbility::ModbusAbility() : unitId(1) {
    for (int i = 0; i < REG_COUNT; i++) {
        holdingRegs[i] = 0;
        inputRegs[i] = 0;
        coils[i] = 0;
        discreteInputs[i] = 0;
    }
}

// CRC16 (Modbus)
static uint16_t modbusCrc(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

CommandOutput modbusAbilityDispatch(void *inst, const char *act, const String &args) {
    ModbusAbility *self = static_cast<ModbusAbility *>(inst);

    if (FE_EQ_P(act, "set_unit_id")) {
        uint8_t id = (uint8_t)args.toInt();
        if (id == 0 || id > 247)
            return CommandOutput{String(act), String(), String(F("invalid unit id"))};
        self->unitId = id;
        return CommandOutput{String(act), String(F("unit_id=")) + id, String()};
    }
    if (FE_EQ_P(act, "get_unit_id")) {
        return CommandOutput{String(act), String(F("unit_id=")) + (int)self->unitId, String()};
    }

    // 参数统一格式：addr[,count]（从 0 开始）
    int comma = args.indexOf(',');
    int addr = (comma > 0 ? args.substring(0, comma) : args).toInt();
    int count = (comma > 0 ? args.substring(comma + 1) : String(F("1"))).toInt();
    if (addr < 0 || count < 0) return CommandOutput{String(act), String(), String(F("bad args"))};

    if (FE_EQ_P(act, "read_holding")) {
        if (addr > ModbusAbility::REG_COUNT - count)
            return CommandOutput{String(act), String(), String(F("addr out of range"))};
        String out = F("[");
        for (int i = 0; i < count; i++) { if (i) out += F(","); out += self->holdingRegs[addr + i]; }
        out += F("]");
        return CommandOutput{String(act), out, String()};
    }
    if (FE_EQ_P(act, "read_input")) {
        if (addr > ModbusAbility::REG_COUNT - count)
            return CommandOutput{String(act), String(), String(F("addr out of range"))};
        String out = F("[");
        for (int i = 0; i < count; i++) { if (i) out += F(","); out += self->inputRegs[addr + i]; }
        out += F("]");
        return CommandOutput{String(act), out, String()};
    }
    if (FE_EQ_P(act, "read_coils")) {
        if (addr > ModbusAbility::REG_COUNT - count)
            return CommandOutput{String(act), String(), String(F("addr out of range"))};
        String out = F("[");
        for (int i = 0; i < count; i++) { if (i) out += F(","); out += self->coils[addr + i] ? '1' : '0'; }
        out += F("]");
        return CommandOutput{String(act), out, String()};
    }
    if (FE_EQ_P(act, "read_discrete")) {
        if (addr > ModbusAbility::REG_COUNT - count)
            return CommandOutput{String(act), String(), String(F("addr out of range"))};
        String out = F("[");
        for (int i = 0; i < count; i++) { if (i) out += F(","); out += self->discreteInputs[addr + i] ? '1' : '0'; }
        out += F("]");
        return CommandOutput{String(act), out, String()};
    }
    if (FE_EQ_P(act, "write_holding")) {
        // 参数：addr,value
        if (comma <= 0) return CommandOutput{String(act), String(), String(F("expect addr,value"))};
        uint16_t value = (uint16_t)args.substring(comma + 1).toInt();
        if (addr >= ModbusAbility::REG_COUNT)
            return CommandOutput{String(act), String(), String(F("addr out of range"))};
        self->holdingRegs[addr] = value;
        return CommandOutput{String(act), String(F("{\"written\":true}")), String()};
    }
    if (FE_EQ_P(act, "write_coil")) {
        if (comma <= 0) return CommandOutput{String(act), String(), String(F("expect addr,value"))};
        bool value = args.substring(comma + 1).toInt() != 0;
        if (addr >= ModbusAbility::REG_COUNT)
            return CommandOutput{String(act), String(), String(F("addr out of range"))};
        self->coils[addr] = value;
        return CommandOutput{String(act), String(F("{\"written\":true}")), String()};
    }

    return CommandOutput{String(act), String(), String(F("unsupported command: ")) + act};
}

} // namespace fe
