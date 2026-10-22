// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_onekey.cpp — OneKeyAbility 实现（Arduino Uno R3 版）
// 令牌 = HMAC-SHA256(secret, "seq:subject")，base64url 呈现。
// 密钥与序列号通过 EEPROM 持久化（ATmega328P 内置 1KB 硬件 EEPROM）。
#include "fe_ability.h"
#include "fe_hmac_sha256.h"
#include <EEPROM.h>
namespace fe {
#ifdef __AVR__
static const char B64URL_TBL[] PROGMEM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
#define B64URL_CHAR(i) ((char)pgm_read_byte(&B64URL_TBL[(i)]))
#else
static const char B64URL_TBL[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
#define B64URL_CHAR(i) (B64URL_TBL[(i)])
#endif
static void b64url_encode(const uint8_t *data, size_t len, char *out, size_t outlen) {
    size_t n = 0;
    for (size_t i = 0; i + 2 < len && n + 4 < outlen; i += 3) {
        uint32_t v = ((uint32_t)data[i] << 16) | ((uint32_t)data[i+1] << 8) | data[i+2];
        out[n++] = B64URL_CHAR((v >> 18) & 63);
        out[n++] = B64URL_CHAR((v >> 12) & 63);
        out[n++] = B64URL_CHAR((v >> 6) & 63);
        out[n++] = B64URL_CHAR(v & 63);
    }
    if (len % 3 == 1 && n + 2 <= outlen) {
        uint32_t v = (uint32_t)data[len-1] << 16;
        out[n++] = B64URL_CHAR((v >> 18) & 63);
        out[n++] = B64URL_CHAR((v >> 12) & 63);
    } else if (len % 3 == 2 && n + 3 <= outlen) {
        uint32_t v = ((uint32_t)data[len-2] << 16) | ((uint32_t)data[len-1] << 8);
        out[n++] = B64URL_CHAR((v >> 18) & 63);
        out[n++] = B64URL_CHAR((v >> 12) & 63);
        out[n++] = B64URL_CHAR((v >> 6) & 63);
    }
    if (n < outlen) out[n] = 0;
}
#define SECRET_ADDR 0
#define SEQ_ADDR    64
// 用浮空 A0 的 ADC 低位噪声 + 上电时间混合播种 random()。
// Arduino random() 未播种时序列固定, 所有设备首次上电会生成相同 HMAC secret,
// 攻击者可离线算出 secret 并伪造任意设备的鉴权 token。
static void seedRandom() {
    uint32_t seed = (uint32_t)millis();
    for (int i = 0; i < 8; i++) {
        seed ^= ((uint32_t)analogRead(A0) & 3u) << (i * 2);
    }
    randomSeed(seed);
}
static String loadOrCreateSecret() {
    char buf[33];
    bool hasSecret = true;
    for (int i = 0; i < 32; i++) {
        buf[i] = (char)EEPROM.read(SECRET_ADDR + i);
        if ((uint8_t)buf[i] == 0xFF) hasSecret = false;
    }
    buf[32] = 0;
    if (!hasSecret) {
        seedRandom();
        for (int i = 0; i < 32; i++) {
            buf[i] = (char)random(256);
            EEPROM.write(SECRET_ADDR + i, (uint8_t)buf[i]);
        }
    }
    return String(buf);
}
static size_t loadSeq() {
    size_t seq = 0;
    for (int i = 0; i < 4; i++) seq |= ((size_t)EEPROM.read(SEQ_ADDR + i)) << (i * 8);
    return seq;
}
static void saveSeq(size_t seq) {
    for (int i = 0; i < 4; i++) EEPROM.write(SEQ_ADDR + i, (uint8_t)(seq >> (i * 8)));
}
CommandOutput oneKeyAbilityDispatch(void *inst, const char *act, const String &args) {
    OneKeyAbility *self = static_cast<OneKeyAbility *>(inst);
    if (self->secret.length() == 0) self->secret = loadOrCreateSecret();
    if (self->tokenSeq == 0) self->tokenSeq = loadSeq();
    String subject = args.length() ? args : String(F("default"));
    if (FE_EQ_P(act, "status")) {
        return CommandOutput{String(act), String(F("{\"tokens\":")) + (self->tokenSeq + 1) + F("}"), String()};
    }
    if (FE_EQ_P(act, "issue_token")) {
        char payload[48];
        uint8_t mac[32];
        char tok[48];
        snprintf_P(payload, sizeof(payload), PSTR("%lu:%s"), (unsigned long)self->tokenSeq, subject.c_str());
        fe_hmac_sha256((const uint8_t *)self->secret.c_str(), self->secret.length(),
                       (const uint8_t *)payload, strlen(payload), mac);
        b64url_encode(mac, 32, tok, sizeof(tok));
        String out = String(F("{\"token\":\"")) + tok + F("\",\"seq\":") + (unsigned long)self->tokenSeq + F("}");
        self->tokenSeq++;
        saveSeq(self->tokenSeq);
        return CommandOutput{String(act), out, String()};
    }
    if (FE_EQ_P(act, "verify_token")) {
        int colon1 = args.indexOf(':');
        if (colon1 <= 0) return CommandOutput{String(act), String(), String(F("bad format, expect seq:token"))};
        String seqStr = args.substring(0, colon1);
        String rest = args.substring(colon1 + 1);
        int colon2 = rest.indexOf(':');
        String tok = colon2 > 0 ? rest.substring(0, colon2) : rest;
        size_t seq = (size_t)seqStr.toInt();
        String subj = (colon2 > 0) ? rest.substring(colon2 + 1) : subject;
        char payload[48];
        uint8_t mac[32];
        char expect[48];
        snprintf_P(payload, sizeof(payload), PSTR("%lu:%s"), (unsigned long)seq, subj.c_str());
        fe_hmac_sha256((const uint8_t *)self->secret.c_str(), self->secret.length(),
                       (const uint8_t *)payload, strlen(payload), mac);
        b64url_encode(mac, 32, expect, sizeof(expect));
        bool valid = (strcmp(expect, tok.c_str()) == 0);
        return valid ? CommandOutput{String(act), String(F("{\"valid\":true}")), String()}
                     : CommandOutput{String(act), String(), String(F("token invalid"))};
    }
    if (FE_EQ_P(act, "revoke_all")) {
        self->tokenSeq = 0; saveSeq(0);
        return CommandOutput{String(act), String(F("{\"revoked\":true}")), String()};
    }
    if (FE_EQ_P(act, "list_tokens")) {
        return CommandOutput{String(act), String(F("{\"tokens\":[]}")), String()};
    }
    if (FE_EQ_P(act, "rotate")) {
        for (int i = 0; i < 32; i++) EEPROM.write(SECRET_ADDR + i, 0xFF);
        self->secret = loadOrCreateSecret();
        self->tokenSeq = 0; saveSeq(0);
        return CommandOutput{String(act), String(F("{\"rotated\":true}")), String()};
    }
    return CommandOutput{String(act), String(), String(F("unsupported command: ")) + act};
}
} // namespace fe
