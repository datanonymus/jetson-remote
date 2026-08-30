#ifndef UDPSENDER_H
#define UDPSENDER_H

#include <QObject>
#include <QUdpSocket>
#include <QString>
#include <QSettings>
#include <QUuid>

class UdpSender : public QObject
{
    Q_OBJECT
public:
    explicit UdpSender(QObject *parent = nullptr);

    Q_INVOKABLE void setTargetIp(const QString &ip);
    Q_INVOKABLE void sendMouseData(int x, int y, int click, int scroll = 0);
    Q_INVOKABLE void sendSignal(int signal, int width, int height, int pin);

    Q_INVOKABLE void sendKeyData(int keycode, int keystate);
    Q_INVOKABLE QString getDeviceId() const {return QString(m_device_id);}

private:
    QUdpSocket *m_socket;
    QString m_targetIp;
    int m_targetPort;
    char m_device_id[32]; // Lưu ID của thiết bị
};

#endif // UDPSENDER_H
