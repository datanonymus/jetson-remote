#include "udpsender.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <cstring>
#include <openssl/evp.h>

// Khóa tĩnh 16-byte
const unsigned char AES_KEY[20] = "DATANONYMUS_KEY_123";
const unsigned char AES_IV[20]  = "DATANONYMUS_IV_4567";

// Hàm thực thi AES-128-CTR (is_encrypt = 1 là Mã hóa, 0 là Giải mã)
int process_aes_ctr(const unsigned char *in_data, int in_len, unsigned char *out_data, int is_encrypt) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0;
    int out_len = 0;
    
    EVP_CipherInit_ex(ctx, EVP_aes_128_ctr(), nullptr, AES_KEY, AES_IV, is_encrypt);
    EVP_CipherUpdate(ctx, out_data, &len, in_data, in_len);
    out_len = len;
    EVP_CipherFinal_ex(ctx, out_data + len, &len);
    out_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    return out_len;
}

// Cấu trúc gói tin y hệt Jetson
struct MouseAndKeyboardPacket {
    int x;
    int y;
    int click;
    int scroll;
    int signal;

    // For keyboard
    int is_keyboard; // 1: Lệnh phím, 0: Lệnh chuột
    int keycode;     // Mã phím cứng (VD: phím A, B, C...)
    int keystate;    // 1: Đang bấm xuống, 0: Nhả phím ra
    char device_id[32]; // ID của Client
};

// Hàm khởi tạo
UdpSender::UdpSender(QObject *parent)
    : QObject(parent), m_targetIp("127.0.0.1"), m_targetPort(5001) // IP mặc định lúc mới mở
{
    m_socket = new QUdpSocket(this);
    QSettings settings("Datanonymus", "JetsonRemoteClient");
    QString deviceId = settings.value("device_id", "").toString();
    if (deviceId.isEmpty()) {
        deviceId = "LAPTOP-" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(6).toUpper();
        settings.setValue("device_id", deviceId);
        qDebug() << "[*] New device detected. Assigned ID: " << deviceId << " for Client.";
    } else {
        qDebug() << "[*] Client ID loaded successfully! Device ID is: " << deviceId;
    }

    std::strncpy(m_device_id, deviceId.toStdString().c_str(), 31);
    m_device_id[31] = '\0';
}

// Hàm cập nhật IP từ giao diện QML
void UdpSender::setTargetIp(const QString &ip) {
    m_targetIp = ip;
    qDebug() << "[+] Updated IP address into:" << m_targetIp;
}

// Hàm điều khiển chuột
void UdpSender::sendMouseData(int x, int y, int click, int scroll)
{
    MouseAndKeyboardPacket packet;
    memset(&packet, 0, sizeof(packet)); // Dọn rác trong RAM
    packet.x = x;
    packet.y = y;
    packet.click = click;
    packet.scroll = scroll;
    packet.is_keyboard = 0; // Mode chuột

    // Chèn ID của thiết bị vào gói tin trước khi gửi đi
    std::strncpy(packet.device_id, m_device_id, 31);
    packet.device_id[31] = '\0';

    unsigned char encrypted_buffer[sizeof(MouseAndKeyboardPacket)];

    process_aes_ctr(reinterpret_cast<const unsigned char*>(&packet), sizeof(packet), encrypted_buffer, 1);

    // Đóng gói và gửi sang Jetson bằng m_targetIp
    m_socket->writeDatagram(reinterpret_cast<const char*>(encrypted_buffer),
                            sizeof(encrypted_buffer),
                            QHostAddress(m_targetIp),
                            m_targetPort);
}

// Hàm signal
void UdpSender::sendSignal(int signal, int width, int height, int pin)
{
    MouseAndKeyboardPacket packet;
    memset(&packet, 0, sizeof(packet)); // Dọn rác trong RAM

    packet.signal = signal;
    packet.x = width;
    packet.y = height;
    packet.keycode = pin; // Lưu trữ mã PIN

    // Chèn ID của thiết bị vào gói tin trước khi gửi đi
    std::strncpy(packet.device_id, m_device_id, 31);
    packet.device_id[31] = '\0';

    unsigned char encrypted_buffer[sizeof(MouseAndKeyboardPacket)];

    process_aes_ctr(reinterpret_cast<const unsigned char*>(&packet), sizeof(packet), encrypted_buffer, 1);

    // Đóng gói và gửi sang Jetson bằng m_targetIp
    m_socket->writeDatagram(reinterpret_cast<const char*>(encrypted_buffer),
                            sizeof(encrypted_buffer),
                            QHostAddress(m_targetIp),
                            m_targetPort);
}

// Hàm nhập bàn phím
void UdpSender::sendKeyData(int keycode, int keystate) {
    MouseAndKeyboardPacket packet;
    memset(&packet, 0, sizeof(packet)); // Dọn sạch rác trong bộ nhớ

    packet.is_keyboard = 1; // Bật cờ khai báo "Tao là bàn phím"
    packet.keycode = keycode;
    packet.keystate = keystate;

    // Chèn ID của thiết bị vào gói tin trước khi gửi đi
    std::strncpy(packet.device_id, m_device_id, 31);
    packet.device_id[31] = '\0';

    unsigned char encrypted_buffer[sizeof(MouseAndKeyboardPacket)];

    process_aes_ctr(reinterpret_cast<const unsigned char*>(&packet), sizeof(packet), encrypted_buffer, 1);

    m_socket->writeDatagram(reinterpret_cast<const char*>(encrypted_buffer),
                            sizeof(encrypted_buffer),
                            QHostAddress(m_targetIp),
                            m_targetPort);
}
