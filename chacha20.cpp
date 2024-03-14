#include "chacha20.h"

Chacha20::Chacha20()
{
    rng = default_random_engine(rd());
    dist = uniform_int_distribution<uint64_t>(0, UINT64_MAX);

    if (!gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P))          {
        if (!gcry_check_version(nullptr))
        {
            fprintf(stderr, "libgcrypt is too old (need %s, have %s)\n",
                    "nullptr", gcry_check_version(NULL));
            exit(2);
        }

        gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    }

}

uint8_t *Chacha20::encrypt(string password, uint8_t *pText, int length, int *cLength)
{
    uint8_t *salt = genSalt();
    uint8_t *pbkdf2 = genPBKDF2(password, salt);

    uint8_t *key = new uint8_t[KEY_LENGTH];
    uint8_t *iv = new uint8_t[IV_LENGTH];

    splitPBKDF2(pbkdf2, key, iv);

    uint8_t *cText = new uint8_t[length];

    encrypt(key, iv, length, cText, pText);

    uint8_t *saltedCText = ByteTools::arrayConCat(salt, SALT_LENGTH, cText, length);
    *cLength = SALT_LENGTH + length;

    delete[] salt;
    delete[] pbkdf2;
    delete[] key;
    delete[] iv;
    delete[] cText;
    return saltedCText;

}

uint8_t *Chacha20::encryptWMagic(string password, uint8_t *pText, int length, int *cLength)
{
    uint8_t *cText = encrypt(password, pText, length, cLength);

    *cLength += MAGIC_VALUE.length();
    uint8_t *ret = ByteTools::arrayConCat((uint8_t *)MAGIC_VALUE.c_str(), MAGIC_VALUE.length(), cText, length + SALT_LENGTH);

    delete[] cText;
    return ret;

}

uint8_t *Chacha20::decrypt(string password, uint8_t *cText, int length, int *pLength)
{
    *pLength = length - SALT_LENGTH;
    if(*pLength > 0){
        uint8_t *pText = new uint8_t[*pLength];
        uint8_t *salt = new uint8_t[SALT_LENGTH];
        ByteTools::arrayCopy(cText, 0, salt, 0, SALT_LENGTH);
        uint8_t *pbkdf2 = genPBKDF2(password, salt);
        uint8_t *key = new uint8_t[KEY_LENGTH];
        uint8_t *iv = new uint8_t[IV_LENGTH];

        splitPBKDF2(pbkdf2, key, iv);

        decrypt(key, iv, *pLength, &cText[SALT_LENGTH], pText);

        delete[] pbkdf2;
        delete[] key;
        delete[] iv;
        delete[] salt;
        return pText;
    }
    else{
        *pLength = -1;
        return nullptr;
    }
}

uint8_t *Chacha20::decryptWMagic(string password, uint8_t *cText, int length, int *pLength)
{
    int offset = MAGIC_VALUE.size();
    *pLength = length - offset;
    if(*pLength > 0){
        string magicText((char *)cText, offset);
        if(magicText.compare(MAGIC_VALUE) == 0){
            return decrypt(password, &cText[offset], length - offset, pLength);
        }
    }


    *pLength = -1;
    return nullptr;

}

void Chacha20::test()
{
    string input = "This is a test file with text.\n";

    uint8_t *pText = new uint8_t[input.length()];
    ByteTools::arrayCopy((uint8_t *)input.c_str(), 0, pText, 0, input.length());
    string passwd = "123456";

    uint8_t *salt = genSalt(true);
    cout << "Salt:\t" << ByteTools::toHex(salt, SALT_LENGTH) << endl;

    uint8_t *pbkdf2 = genPBKDF2(passwd, salt);

    cout << "PBKDF2:\t" << ByteTools::toHex(pbkdf2, (KEY_LENGTH + IV_LENGTH)) << endl;


    uint8_t *key = new uint8_t[KEY_LENGTH];
    uint8_t *iv = new uint8_t[IV_LENGTH];

    splitPBKDF2(pbkdf2, key, iv);
    cout << "Key:\t" << ByteTools::toHex(key, KEY_LENGTH) << endl;
    //arrayCopy(pbkdf2, KEY_LENGTH, iv, 0, IV_LENGTH);
    cout << "IV:\t" << ByteTools::toHex(iv, IV_LENGTH) << endl;

    size_t len = input.length();
    uint8_t *cText = new uint8_t[len];

    encrypt(key, iv, len, cText, pText);

    cout << "cText:\t" << ByteTools::toHex(cText, len) << endl;

    uint8_t *saltedCText = ByteTools::arrayConCat(salt, SALT_LENGTH, cText, len);
    cout << "Salted cT:\t" << ByteTools::toHex(saltedCText, SALT_LENGTH + len) << endl;

    int cLength = 0;
    uint8_t *enc = encrypt(passwd, pText, len, &cLength);
    cout << "Encrypt cT:\t" << ByteTools::toHex(enc, cLength) << endl;

    uint8_t *paddedCText = ByteTools::arrayConCat((uint8_t *)MAGIC_VALUE.c_str(), MAGIC_VALUE.length(), saltedCText, SALT_LENGTH + len );
    cout << "Padded cT:\t" << ByteTools::toHex(paddedCText, SALT_LENGTH + len + MAGIC_VALUE.length()) << endl;

    uint8_t *encMag = encryptWMagic(passwd, pText, len, &cLength);
    cout << "Encrypt cT:\t" << ByteTools::toHex(encMag, cLength) << endl;


    cout << endl << endl << "*** Decryption ***" << endl;
    int pLen = 0;
    uint8_t *decryptedCText = decrypt(passwd, saltedCText, SALT_LENGTH + len, &pLen);
    cout << "Decrypted Salted cT:\t" << ByteTools::toHex(decryptedCText, pLen) << endl;
    string word((char *)decryptedCText, pLen);
    cout << "As string: \t" << word << endl;

    uint8_t *decryptedWCText = decryptWMagic(passwd, encMag, cLength, &pLen);
    cout << "Decrypted Salted cT:\t" << ByteTools::toHex(decryptedWCText, pLen) << endl;
    string wordW((char *)decryptedWCText, pLen);
    cout << "As string: \t" << wordW << endl;


    delete[] salt;
    delete[] pbkdf2;
    delete[] key;
    delete[] iv;
    delete[] cText;
    delete[] pText;
    delete[] saltedCText;
    delete[] paddedCText;
    delete[] enc;
    delete[] encMag;
    delete[] decryptedCText;
    delete[] decryptedWCText;
}

uint8_t *Chacha20::genSalt(bool std)
{
    uint8_t *ret = new uint8_t[SALT_LENGTH];
    uint64_t salt = dist(rng);
    uint8_t stdSalt[8] = { 0x76, 0x5C, 0xEA, 0x76, 0xDD, 0x7D, 0xDA, 0xDE};
    if(std){
        ByteTools::arrayCopy((uint8_t *)&stdSalt, 0, ret, 0, SALT_LENGTH);
    }
    else{
        ByteTools::arrayCopy((uint8_t *)&salt, 0, ret, 0, SALT_LENGTH);
    }
    return ret;
}

uint8_t *Chacha20::genPBKDF2(string password, uint8_t *salt)
{

    int len = (KEY_LENGTH + IV_LENGTH);
    uint8_t *ret = new uint8_t[len];
    gcry_kdf_derive(password.c_str(), password.length(), GCRY_KDF_PBKDF2, GCRY_MD_SHA256, salt, SALT_LENGTH, ITER, len, ret);
    return ret;
}

void Chacha20::splitPBKDF2(uint8_t *pbkdf2, uint8_t *key, uint8_t *iv)
{
    ByteTools::arrayCopy(pbkdf2, 0, key, 0, KEY_LENGTH);
    ByteTools::arrayCopy(pbkdf2, KEY_LENGTH, iv, 0, IV_LENGTH);
}

void Chacha20::encrypt(uint8_t *key, uint8_t *iv, size_t len, uint8_t *cText, uint8_t *pText)
{
    gcry_cipher_hd_t handle;
    gcry_error_t err = 0;

    //select cypher
    err = gcry_cipher_open (&handle, GCRY_CIPHER_CHACHA20, GCRY_CIPHER_MODE_STREAM,0);
    if (err){
        fprintf (stderr, "Select Failure: %s/%s\n",
                 gcry_strsource (err),
                 gcry_strerror (err));
    }
    else{ // set Key
        err = gcry_cipher_setkey(handle, (const void *)key, KEY_LENGTH);
        if (err){
            fprintf (stderr, "Key Failure: %s/%s\n",
                     gcry_strsource (err),
                     gcry_strerror (err));

        }
        else{ // set iv
            err = gcry_cipher_setiv(handle, (const void *)iv, IV_LENGTH);
            if (err){
                fprintf (stderr, "Nonce Failure: %s/%s\n",
                         gcry_strsource (err),
                         gcry_strerror (err));

            }
            else{// encrypt
                err = gcry_cipher_encrypt(handle, cText, len, pText, len);
                if (err){
                    fprintf (stderr, "Nonce Failure: %s/%s\n",
                             gcry_strsource (err),
                             gcry_strerror (err));

                }
            }
        }
    }
}

void Chacha20::decrypt(uint8_t *key, uint8_t *iv, size_t len, uint8_t *cText, uint8_t *pText)
{
    gcry_cipher_hd_t handle;
    gcry_error_t err = 0;

    //select cypher
    err = gcry_cipher_open (&handle, GCRY_CIPHER_CHACHA20, GCRY_CIPHER_MODE_STREAM,0);
    if (err){
        fprintf (stderr, "Select Failure: %s/%s\n",
                 gcry_strsource (err),
                 gcry_strerror (err));
    }
    else{ // set Key
        err = gcry_cipher_setkey(handle, (const void *)key, KEY_LENGTH);
        if (err){
            fprintf (stderr, "Key Failure: %s/%s\n",
                     gcry_strsource (err),
                     gcry_strerror (err));

        }
        else{ // set iv
            err = gcry_cipher_setiv(handle, (const void *)iv, IV_LENGTH);
            if (err){
                fprintf (stderr, "Nonce Failure: %s/%s\n",
                         gcry_strsource (err),
                         gcry_strerror (err));

            }
            else{// encrypt
                err = gcry_cipher_decrypt(handle, pText, len, cText, len);
                if (err){
                    fprintf (stderr, "Nonce Failure: %s/%s\n",
                             gcry_strsource (err),
                             gcry_strerror (err));

                }
            }
        }
    }
}


