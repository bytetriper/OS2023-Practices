#ifndef CRYPTO_H
#define CRYPTO_H
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>

#include <openssl/rsa.h>
#include <openssl/pem.h>
void generate_key(RSA*& rsa, std::string& public_key, std::string& private_key);
std::string encrypt(std::string plaintext, RSA* rsa);
std::string decrypt(std::string ciphertext, RSA* rsa);
RSA* create_rsa_from_key(std::string key_str, bool is_public_key);
#endif
