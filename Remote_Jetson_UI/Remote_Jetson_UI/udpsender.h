#ifndef UDPSENDER_H
#define UDPSENDER_H

#include <QObject>
#include <QUdpSocket>
#include <QString>
#include <QSettings>
#include <QUuid>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>

class UdpSender : public QObject
{
    Q_OBJECT
signals:
    // Signal báo cáo kết quả về cho QML ("trusted", "unknown", hoặc "error")
    void authStatusReceived(QString status);
public:
    explicit UdpSender(QObject *parent = nullptr);

    Q_INVOKABLE void setTargetIp(const QString &ip);
    Q_INVOKABLE void sendMouseData(int x, int y, int click, int scroll = 0);
    Q_INVOKABLE void sendSignal(int signal, int width, int height, int pin);

    Q_INVOKABLE void sendKeyData(int keycode, int keystate);
    Q_INVOKABLE QString getDeviceId() const {return QString(m_device_id);}

    Q_INVOKABLE void checkAndFetchKey(QString target_ip, QString my_device_id);
    Q_INVOKABLE void clearAesKey();

private:
    QUdpSocket *m_socket;
    QString m_targetIp;
    QByteArray m_currentAesKey; // Biến lưu Key của Client đang bật hiện tại
    int m_targetPort;
    char m_device_id[32]; // Lưu ID của thiết bị
};

#endif // UDPSENDER_H
