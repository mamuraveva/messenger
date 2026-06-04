#pragma once

#include <vector>

/**
 * @brief Отправляет пакет данных через сокет.
 *
 * Перед отправкой к данным добавляется их размер,
 * что позволяет принимающей стороне определить границы пакета.
 *
 * @param socket Дескриптор сокета.
 * @param data Передаваемые данные.
 * @return true если пакет успешно отправлен.
 * @return false в случае ошибки.
 */
bool send_packet(int socket, const std::vector<unsigned char> &data);

/**
 * @brief Получает пакет данных из сокета.
 *
 * Сначала считывает размер пакета, затем получает
 * соответствующее количество байтов.
 *
 * @param socket Дескриптор сокета.
 * @param data Буфер для сохранения полученных данных.
 * @return true если пакет успешно получен.
 * @return false в случае ошибки или разрыва соединения.
 */
bool recv_packet(int socket, std::vector<unsigned char> &data);