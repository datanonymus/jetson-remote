#include "udpsender.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <cstring>

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
        qDebug() << "[*] Phát hiện thiết bị mới. Cấp ID: " << deviceId << " cho Client.";
    } else {
        qDebug() << "[*] Load ID Client thành công. ID của thiết bị là: " << deviceId;
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

    // Đóng gói và gửi sang Jetson bằng m_targetIp
    m_socket->writeDatagram(reinterpret_cast<const char*>(&packet),
                            sizeof(MouseAndKeyboardPacket),
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

    // Đóng gói và gửi sang Jetson bằng m_targetIp
    m_socket->writeDatagram(reinterpret_cast<const char*>(&packet),
                            sizeof(MouseAndKeyboardPacket),
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

    m_socket->writeDatagram(reinterpret_cast<const char*>(&packet),
                            sizeof(MouseAndKeyboardPacket),
                            QHostAddress(m_targetIp),
                            m_targetPort);
}
