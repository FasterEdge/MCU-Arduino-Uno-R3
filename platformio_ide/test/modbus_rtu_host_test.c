#include "fe_ability.h"
#include "fe_port.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static u8 tx[128];
static u16 tx_len;

u16 fe_port_uart_write(u8 port, const u8 *data, u16 len) {
    (void)port;
    assert(len <= sizeof(tx));
    memcpy(tx, data, len);
    tx_len = len;
    return len;
}

int fe_snprintf(char *buf, u16 size, const char *fmt, ...) {
    int n;
    va_list ap;
    va_start(ap, fmt);
    n = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return n;
}

fe_output_t fe_ok(const char *name, const char *value) {
    fe_output_t out;
    memset(&out, 0, sizeof(out));
    out.ok = 1;
    snprintf(out.name, sizeof(out.name), "%s", name ? name : "");
    snprintf(out.value, sizeof(out.value), "%s", value ? value : "");
    return out;
}

fe_output_t fe_err(const char *name, const char *err) {
    fe_output_t out = fe_ok(name, "");
    out.ok = 0;
    snprintf(out.err, sizeof(out.err), "%s", err ? err : "");
    return out;
}

static u16 crc16(const u8 *data, u16 len) {
    u16 crc = 0xFFFF;
    u16 i;
    u8 bit;
    for (i = 0; i < len; ++i) {
        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit)
            crc = (crc & 1) ? (u16)((crc >> 1) ^ 0xA001) : (u16)(crc >> 1);
    }
    return crc;
}

static void finish_request(u8 frame[8]) {
    u16 crc = crc16(frame, 6);
    frame[6] = (u8)crc;
    frame[7] = (u8)(crc >> 8);
}

static void request(modbus_ability_t *m, u8 unit, u8 fn, u16 addr, u16 value) {
    u8 frame[8];
    frame[0] = unit; frame[1] = fn;
    frame[2] = (u8)(addr >> 8); frame[3] = (u8)addr;
    frame[4] = (u8)(value >> 8); frame[5] = (u8)value;
    finish_request(frame);
    tx_len = 0;
    modbus_slave_service(m, frame, sizeof(frame));
}

static void expect_tx(const u8 *pdu, u16 pdu_len) {
    u16 crc;
    assert(tx_len == pdu_len + 2);
    assert(memcmp(tx, pdu, pdu_len) == 0);
    crc = crc16(tx, pdu_len);
    assert(tx[pdu_len] == (u8)crc);
    assert(tx[pdu_len + 1] == (u8)(crc >> 8));
}

int main(void) {
    modbus_ability_t m;
    u8 expected[70];
    u8 bad_crc[8] = {1, 3, 0, 0, 0, 1, 0, 0};
    u8 i;
    memset(&m, 0, sizeof(m));
    m.unit_id = 1;
    m.holding_regs[1] = 0x1234;
    m.holding_regs[2] = 0xABCD;
    m.input_regs[3] = 0x5678;
    m.coils[0] = 1; m.coils[2] = 1; m.coils[8] = 1;
    m.discrete_inputs[1] = 1; m.discrete_inputs[7] = 1;

    request(&m, 1, 3, 1, 2);
    { const u8 p[] = {1, 3, 4, 0x12, 0x34, 0xAB, 0xCD}; expect_tx(p, sizeof(p)); }
    request(&m, 1, 4, 3, 1);
    { const u8 p[] = {1, 4, 2, 0x56, 0x78}; expect_tx(p, sizeof(p)); }
    request(&m, 1, 1, 0, 9);
    { const u8 p[] = {1, 1, 2, 0x05, 0x01}; expect_tx(p, sizeof(p)); }
    request(&m, 1, 2, 0, 8);
    { const u8 p[] = {1, 2, 1, 0x82}; expect_tx(p, sizeof(p)); }

    request(&m, 1, 5, 4, 0xFF00);
    assert(m.coils[4] == 1);
    { const u8 p[] = {1, 5, 0, 4, 0xFF, 0}; expect_tx(p, sizeof(p)); }
    request(&m, 1, 6, 5, 0xCAFE);
    assert(m.holding_regs[5] == 0xCAFE);
    { const u8 p[] = {1, 6, 0, 5, 0xCA, 0xFE}; expect_tx(p, sizeof(p)); }

    request(&m, 1, 0x10, 0, 1);
    { const u8 p[] = {1, 0x90, 1}; expect_tx(p, sizeof(p)); }
    request(&m, 1, 3, 31, 2);
    { const u8 p[] = {1, 0x83, 2}; expect_tx(p, sizeof(p)); }
    request(&m, 1, 5, 0, 1);
    { const u8 p[] = {1, 0x85, 3}; expect_tx(p, sizeof(p)); }

    request(&m, 0, 6, 6, 0xBEEF);
    assert(m.holding_regs[6] == 0xBEEF && tx_len == 0);
    request(&m, 2, 6, 6, 0x1111);
    assert(m.holding_regs[6] == 0xBEEF && tx_len == 0);
    tx_len = 0;
    modbus_slave_service(&m, bad_crc, sizeof(bad_crc));
    assert(tx_len == 0);

    request(&m, 1, 3, 0, 32);
    assert(tx_len == 69);
    expected[0] = 1; expected[1] = 3; expected[2] = 64;
    for (i = 0; i < 32; ++i) {
        expected[3 + i * 2] = (u8)(m.holding_regs[i] >> 8);
        expected[4 + i * 2] = (u8)m.holding_regs[i];
    }
    expect_tx(expected, 67);

    puts("modbus_rtu_host_test: PASS");
    return 0;
}
