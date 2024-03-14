#ifndef CHACHA20_H
#define CHACHA20_H

#include <gcrypt.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <random>

#include "bytetools.h"

using namespace std;

class Chacha20
{
public:
    Chacha20();
    uint8_t *encrypt(string password, uint8_t *pText, int length, int *cLength);
    uint8_t *encryptWMagic(string password, uint8_t *pText, int length, int *cLength);

    uint8_t *decrypt(string password, uint8_t *cText, int length, int *pLength);
    uint8_t *decryptWMagic(string password, uint8_t *cText, int length, int *pLength);
    void test();

private:
    uint8_t *genSalt(bool std = false);
    uint8_t *genPBKDF2(string password, uint8_t *salt);

    void splitPBKDF2(uint8_t *pbkdf2, uint8_t *key, uint8_t *iv);
    void encrypt(uint8_t *key, uint8_t *iv, size_t len, uint8_t *cText, uint8_t *pText);
    void decrypt(uint8_t *key, uint8_t *iv, size_t len, uint8_t *cText, uint8_t *pText);

    size_t SALT_LENGTH = 8;
    string MAGIC_VALUE = "Salted__";
    unsigned long ITER = 10000;
    size_t KEY_LENGTH = 256 / 8;
    size_t IV_LENGTH = 128 / 8;

    random_device rd;
    default_random_engine rng;
    uniform_int_distribution<uint64_t> dist;
};

#endif // CHACHA20_H
