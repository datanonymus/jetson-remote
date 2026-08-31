#include "udpsender.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Struct tự động dọn rác
struct AesContextWrapper {
    EVP_CIPHER_CTX* ctx;
    AesContextWrapper() {
        ctx = EVP_CIPHER_CTX_new();
    }
    ~AesContextWrapper() {
        // Tự động dọn rác khi luồng kết thúc hoặc app tắt
        if (ctx) EVP_CIPHER_CTX_free(ctx);
    }
};

// Hàm thực thi AES-128-CTR (is_encrypt = 1 là Mã hóa, 0 là Giải mã)
int process_aes_ctr(const unsigned char *in_data, int in_len, unsigned char *out_data, const unsigned char *aes_key, const unsigned char *aes_iv, int is_encrypt) {
    thread_local AesContextWrapper wrapper; 
    int len = 0;
    EVP_CipherInit_ex(wrapper.ctx, EVP_aes_128_ctr(), nullptr, aes_key, aes_iv, is_encrypt);
    EVP_CipherUpdate(wrapper.ctx, out_data, &len, in_data, in_len);
    EVP_CipherFinal_ex(wrapper.ctx, out_data + len, &len);
    
    return in_len;
}

// Cấu trúc gói tin y hệt Jetson
struct MouseAndKeyboardPacket {
    int x, y, click, scroll;
    int signal;

    // For keyboard
    int is_keyboard; // 1: Lệnh phím, 0: Lệnh chuột
    int keycode;     // Mã phím cứng (VD: phím A, B, C...)
    int keystate;    // 1: Đang bấm xuống, 0: Nhả phím ra

    char iv[16]; // AES IV Auto-gen
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

void UdpSender::checkAndFetchKey(QString target_ip, QString my_device_id) {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString url = "http://" + target_ip + ":8080/api/check_auth?id=" + my_device_id;
    QNetworkRequest request((QUrl(url)));
    
    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            QJsonObject jsonObj = jsonDoc.object();
            
            QString status = jsonObj["status"].toString();
            
            if (status == "trusted") {
                // Lấy Key về, đưa vào RAM
                m_currentAesKey = jsonObj["aes_key"].toString().toUtf8();
                
                // Lưu vào Registry/conf
                QSettings settings("Datanonymus", "Jetson_Remote_Client");
                settings.setValue("AES_KEY_" + my_device_id, jsonObj["aes_key"].toString());
                
                qDebug() << "[+] Saved Key successfully!";
                
                // Báo cho QML biết là xong rồi
                emit authStatusReceived("trusted");
            } 
            else if (status == "unknown") {
                emit authStatusReceived("unknown"); // Thiết bị lạ, QML cần hiện PIN
            }
        } else {
            qDebug() << "[!] Network error occured when calling API Jetson.";
            emit authStatusReceived("error");
        }
        
        reply->deleteLater();
        manager->deleteLater();
    });
    
    manager->get(request);
}

void UdpSender::clearAesKey() {
    m_currentAesKey.clear();
    qDebug() << "[+] Deleted AES Key on RAM!";
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

    // Sinh ngẫu nhiên 16 byte IV cho riêng gói tin này
    unsigned char random_iv[16];
    RAND_bytes(random_iv, 16);
    memcpy(packet.iv, random_iv, 16);

    // Chèn ID của thiết bị vào gói tin trước khi gửi đi
    std::strncpy(packet.device_id, m_device_id, 31);
    packet.device_id[31] = '\0';

    // Chuẩn bị mảng byte bằng đúng kích thước struct (80 byte)
    unsigned char raw_buffer[sizeof(MouseAndKeyboardPacket)];
    memcpy(raw_buffer, &packet, sizeof(MouseAndKeyboardPacket));

    QSettings settings("Datanonymus", "Jetson_Remote_Client");
    QString savedKey = settings.value("AES_KEY_" + QString(m_device_id), "").toString();
    QByteArray keyBytes = savedKey.toUtf8();
    const unsigned char* aes_key_ptr = reinterpret_cast<const unsigned char*>(keyBytes.constData());

    // Chỉ mã hóa 32 bytes đầu, sử dụng AES_KEY đã lưu sẵn và AES_IV được tạo ra trước đó
    process_aes_ctr(raw_buffer, 32, raw_buffer, aes_key_ptr, random_iv, 1);

    // Đóng gói và gửi sang Jetson bằng m_targetIp
    m_socket->writeDatagram(reinterpret_cast<const char*>(raw_buffer),
                            sizeof(raw_buffer),
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

    // Sinh ngẫu nhiên 16 byte IV cho riêng gói tin này
    unsigned char random_iv[16];
    RAND_bytes(random_iv, 16);
    memcpy(packet.iv, random_iv, 16);

    // Chèn ID của thiết bị vào gói tin trước khi gửi đi
    std::strncpy(packet.device_id, m_device_id, 31);
    packet.device_id[31] = '\0';

    // Chuẩn bị mảng byte bằng đúng kích thước struct (80 byte)
    unsigned char raw_buffer[sizeof(MouseAndKeyboardPacket)];
    memcpy(raw_buffer, &packet, sizeof(MouseAndKeyboardPacket));

    QSettings settings("Datanonymus", "Jetson_Remote_Client");
    QString savedKey = settings.value("AES_KEY_" + QString(m_device_id), "").toString();
    if (!savedKey.isEmpty()) {
        QByteArray keyBytes = savedKey.toUtf8();
        const unsigned char* aes_key_ptr = reinterpret_cast<const unsigned char*>(keyBytes.constData());

        // Chỉ mã hóa 32 bytes đầu, sử dụng AES_KEY đã lưu sẵn và AES_IV được tạo ra trước đó
        process_aes_ctr(raw_buffer, 32, raw_buffer, aes_key_ptr, random_iv, 1);
    }

    // Đóng gói và gửi sang Jetson bằng m_targetIp
    m_socket->writeDatagram(reinterpret_cast<const char*>(raw_buffer),
                            sizeof(raw_buffer),
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

    // Sinh ngẫu nhiên 16 byte IV cho riêng gói tin này
    unsigned char random_iv[16];
    RAND_bytes(random_iv, 16);
    memcpy(packet.iv, random_iv, 16);

    // Chèn ID của thiết bị vào gói tin trước khi gửi đi
    std::strncpy(packet.device_id, m_device_id, 31);
    packet.device_id[31] = '\0';

    // Chuẩn bị mảng byte bằng đúng kích thước struct (80 byte)
    unsigned char raw_buffer[sizeof(MouseAndKeyboardPacket)];
    memcpy(raw_buffer, &packet, sizeof(MouseAndKeyboardPacket));

    QSettings settings("Datanonymus", "Jetson_Remote_Client");
    QString savedKey = settings.value("AES_KEY_" + QString(m_device_id), "").toString();
    QByteArray keyBytes = savedKey.toUtf8();
    const unsigned char* aes_key_ptr = reinterpret_cast<const unsigned char*>(keyBytes.constData());

    // Chỉ mã hóa 32 bytes đầu, sử dụng AES_KEY đã lưu sẵn và AES_IV được tạo ra trước đó
    process_aes_ctr(raw_buffer, 32, raw_buffer, aes_key_ptr, random_iv, 1);

    m_socket->writeDatagram(reinterpret_cast<const char*>(raw_buffer),
                            sizeof(raw_buffer),
                            QHostAddress(m_targetIp),
                            m_targetPort);
}
