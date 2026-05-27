#pragma once
#include <sodium.h>
#include <vector>
#include <string>
#include <stdexcept>

class Crypto {
private:
    unsigned char pub_key_[crypto_kx_PUBLICKEYBYTES];
    unsigned char priv_key_[crypto_kx_SECRETKEYBYTES];
    unsigned char rx_[crypto_kx_SESSIONKEYBYTES];
    unsigned char tx_[crypto_kx_SESSIONKEYBYTES];
    bool ready_ = false;
public:
    Crypto() {
        if (sodium_init() < 0) {
            throw std::runtime_error("Ошибка инициализации libsodium");
        }
        crypto_kx_keypair(pub_key_, priv_key_);
    }
    const unsigned char* pub_key() const {return pub_key_;}
    void setup_as_dialer(const unsigned char* their_pub) {
        if (crypto_kx_client_session_keys(rx_, tx_, pub_key_, priv_key_, their_pub) != 0) {
            throw std::runtime_error("Ошибка: не удалось выработать общий ключ");
        }
    ready_ = true;
    }
    void setup_as_receiver(const unsigned char* their_pub) {
        if (crypto_kx_server_session_keys(rx_, tx_, pub_key_, priv_key_, their_pub) != 0) {
             throw std::runtime_error("Ошибка: не удалось выработать общий ключ");
        }
    ready_ = true;
    }
    std::vector<unsigned char> encrypt(const std::string& plaintext) {
        if (!ready_) {
            throw std::runtime_error("Ошибка: ключи ещё не выработаны");
        }
        unsigned char nonce[crypto_secretbox_NONCEBYTES];
        randombytes_buf(nonce, sizeof(nonce));
        std::vector<unsigned char> cipher(crypto_secretbox_MACBYTES + plaintext.size());
            crypto_secretbox_easy(cipher.data(), reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size(), nonce, tx_);
            std::vector<unsigned char> result(sizeof(nonce) + cipher.size());
            std::memcpy(result.data(), nonce, sizeof(nonce));
            std::memcpy(result.data() + sizeof(nonce), cipher.data(), cipher.size());
            return result;
    }
    std::string decrypt(const std::vector<unsigned char>& data) {
        if (!ready_) {
            throw std::runtime_error("Ошибка: ключи еще не выработаны");
        }
        size_t min_size = crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES;
        if (data.size() < min_size) {
            throw std::runtime_error("Ошибка: пакет слишком короткий");
        }
        const unsigned char* nonce = data.data();
        const unsigned char* cipher = data.data() + crypto_secretbox_NONCEBYTES;
        size_t cipher_len = data.size() - crypto_secretbox_NONCEBYTES;
        std::vector<unsigned char> plain(cipher_len - crypto_secretbox_MACBYTES);
        if (crypto_secretbox_open_easy(plain.data(), cipher, cipher_len, nonce, rx_) != 0) {
            throw std::runtime_error("Ошибка: расшифровка не удалась — пакет повреждён или подделан");
        }
        return std::string(plain.begin(), plain.end());
    }
};   