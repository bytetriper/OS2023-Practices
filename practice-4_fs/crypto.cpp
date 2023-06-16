#include"crypto.h"

//生成RSA公私钥
void generate_key(RSA*& rsa, std::string& public_key, std::string& private_key) {
    int bits = 1024; //RSA密钥长度为1024位

    BIGNUM* e_bn = BN_new();
    BN_set_word(e_bn, RSA_F4);
    rsa = RSA_new();
    RSA_generate_key_ex(rsa, bits, e_bn, NULL);

    BIO* public_bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPublicKey(public_bio, rsa);
    BUF_MEM* public_buf = NULL;
    BIO_get_mem_ptr(public_bio, &public_buf);
    public_key = std::string(public_buf->data, public_buf->length);
    BIO_free_all(public_bio);

    BIO* private_bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(private_bio, rsa, NULL, NULL, 0, NULL, NULL);
    BUF_MEM* private_buf = NULL;
    BIO_get_mem_ptr(private_bio, &private_buf);
    private_key = std::string(private_buf->data, private_buf->length);
    BIO_free_all(private_bio);
}

//从字符串形式的密钥生成RSA对象
RSA* create_rsa_from_key(std::string key_str, bool is_public_key) {
    RSA* rsa = RSA_new();

    BIO* bio = BIO_new(BIO_s_mem());
    BIO_puts(bio, key_str.c_str());

    if (is_public_key) {
        PEM_read_bio_RSAPublicKey(bio, &rsa, NULL, NULL);
    }
    else {
        PEM_read_bio_RSAPrivateKey(bio, &rsa, NULL, NULL);
    }

    BIO_free_all(bio);

    return rsa;
}

//加密
std::string encrypt(std::string plaintext, RSA* rsa) {
    int rsa_len = RSA_size(rsa);
    unsigned char* ciphertext = new unsigned char[rsa_len];
    RSA_public_encrypt(plaintext.length(), (const unsigned char*)plaintext.c_str(), ciphertext, rsa, RSA_PKCS1_PADDING);
    std::string result((char*)ciphertext, rsa_len);
    delete[] ciphertext;
    return result;
}

//解密
std::string decrypt(std::string ciphertext, RSA* rsa) {
    int rsa_len = RSA_size(rsa);
    unsigned char* plaintext = new unsigned char[rsa_len];
    RSA_private_decrypt(rsa_len, (const unsigned char*)ciphertext.c_str(), plaintext, rsa, RSA_PKCS1_PADDING);
    std::string result((char*)plaintext);
    delete[] plaintext;
    return result;
}

//主函数，演示RSA加密解密流程
/*
int main() {
    RSA* rsa = NULL;
    std::string public_key, private_key;
    std::string plaintext = "Hello, World!";

    generate_key(rsa, public_key, private_key);
    cout << "Public Key: " << public_key << endl;
    cout << "Private Key: " << private_key << endl;

    RSA* rsa_from_public_key = create_rsa_from_key(public_key, true);
    RSA* rsa_from_private_key = create_rsa_from_key(private_key, false);
    std::string ciphertext = encrypt(plaintext, rsa);
    cout << "Ciphertext: " << ciphertext << endl;

    
    std::string decrypted_text = decrypt(ciphertext, rsa);
    cout << "Decrypted Text: " << decrypted_text << endl;

    RSA_free(rsa);
    RSA_free(rsa_from_public_key);
    RSA_free(rsa_from_private_key);

    return 0;
}
*/