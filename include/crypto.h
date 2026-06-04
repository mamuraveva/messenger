#pragma once

#include <cstring>
#include <sodium.h>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Класс для обмена ключами, шифрования и расшифровки сообщений.
 *
 * Использует библиотеку libsodium. Сначала создаётся пара ключей,
 * затем через setup_as_dialer() или setup_as_receiver() вырабатываются
 * сессионные ключи. После этого можно шифровать и расшифровывать сообщения.
 */
class Crypto {
private:
  /** @brief Публичный ключ текущего клиента. */
  unsigned char pub_key_[crypto_kx_PUBLICKEYBYTES];

  /** @brief Приватный ключ текущего клиента. */
  unsigned char priv_key_[crypto_kx_SECRETKEYBYTES];

  /** @brief Ключ для расшифровки входящих сообщений. */
  unsigned char rx_[crypto_kx_SESSIONKEYBYTES];

  /** @brief Ключ для шифрования исходящих сообщений. */
  unsigned char tx_[crypto_kx_SESSIONKEYBYTES];

  /** @brief Показывает, были ли успешно выработаны сессионные ключи. */
  bool ready_ = false;

public:
  /**
   * @brief Создаёт объект Crypto и генерирует пару ключей.
   *
   * Инициализирует libsodium и создаёт публичный и приватный ключи
   * для последующего обмена ключами.
   *
   * @throw std::runtime_error если libsodium не удалось инициализировать.
   */
  Crypto() {
    if (sodium_init() < 0) {
      throw std::runtime_error("Ошибка инициализации libsodium");
    }

    crypto_kx_keypair(pub_key_, priv_key_);
  }

  /**
   * @brief Возвращает публичный ключ клиента.
   *
   * @return Указатель на массив байтов публичного ключа.
   */
  const unsigned char *pub_key() const { return pub_key_; }

  /**
   * @brief Настраивает сессионные ключи для клиента-инициатора.
   *
   * Используется стороной, которая первой начинает обмен ключами.
   *
   * @param their_pub Публичный ключ второго клиента.
   * @throw std::runtime_error если сессионные ключи не удалось выработать.
   */
  void setup_as_dialer(const unsigned char *their_pub) {
    if (crypto_kx_client_session_keys(rx_, tx_, pub_key_, priv_key_,
                                      their_pub) != 0) {
      throw std::runtime_error("Ошибка: не удалось выработать общий ключ");
    }

    ready_ = true;
  }

  /**
   * @brief Настраивает сессионные ключи для клиента-получателя.
   *
   * Используется второй стороной обмена ключами.
   *
   * @param their_pub Публичный ключ первого клиента.
   * @throw std::runtime_error если сессионные ключи не удалось выработать.
   */
  void setup_as_receiver(const unsigned char *their_pub) {
    if (crypto_kx_server_session_keys(rx_, tx_, pub_key_, priv_key_,
                                      their_pub) != 0) {
      throw std::runtime_error("Ошибка: не удалось выработать общий ключ");
    }

    ready_ = true;
  }

  /**
   * @brief Шифрует текстовое сообщение.
   *
   * Генерирует случайный nonce, шифрует сообщение с помощью tx_ и
   * возвращает пакет в формате: nonce + ciphertext.
   *
   * @param plaintext Исходный текст сообщения.
   * @return Вектор байтов с зашифрованным сообщением.
   * @throw std::runtime_error если сессионные ключи ещё не выработаны.
   */
  std::vector<unsigned char> encrypt(const std::string &plaintext) {
    if (!ready_) {
      throw std::runtime_error("Ошибка: ключи ещё не выработаны");
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    std::vector<unsigned char> cipher(crypto_secretbox_MACBYTES +
                                      plaintext.size());

    crypto_secretbox_easy(
        cipher.data(),
        reinterpret_cast<const unsigned char *>(plaintext.data()),
        plaintext.size(), nonce, tx_);

    std::vector<unsigned char> result(sizeof(nonce) + cipher.size());
    std::memcpy(result.data(), nonce, sizeof(nonce));
    std::memcpy(result.data() + sizeof(nonce), cipher.data(), cipher.size());

    return result;
  }

  /**
   * @brief Расшифровывает зашифрованный пакет.
   *
   * Ожидает пакет в формате: nonce + ciphertext. Если пакет повреждён,
   * подделан или слишком короткий, выбрасывает исключение.
   *
   * @param data Вектор байтов с зашифрованным сообщением.
   * @return Расшифрованный текст сообщения.
   * @throw std::runtime_error если ключи не выработаны, пакет слишком короткий
   * или расшифровка не удалась.
   */
  std::string decrypt(const std::vector<unsigned char> &data) {
    if (!ready_) {
      throw std::runtime_error("Ошибка: ключи еще не выработаны");
    }

    size_t min_size = crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES;

    if (data.size() < min_size) {
      throw std::runtime_error("Ошибка: пакет слишком короткий");
    }

    const unsigned char *nonce = data.data();
    const unsigned char *cipher = data.data() + crypto_secretbox_NONCEBYTES;
    size_t cipher_len = data.size() - crypto_secretbox_NONCEBYTES;

    std::vector<unsigned char> plain(cipher_len - crypto_secretbox_MACBYTES);

    if (crypto_secretbox_open_easy(plain.data(), cipher, cipher_len, nonce,
                                   rx_) != 0) {
      throw std::runtime_error(
          "Ошибка: расшифровка не удалась — пакет повреждён или подделан");
    }

    return std::string(plain.begin(), plain.end());
  }
};