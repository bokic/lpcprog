#ifndef QLPCPROG_H
#define QLPCPROG_H

#include <QSerialPort>
#include <QStringList>
#include <QObject>

class QLpcProg : public QObject
{
    Q_OBJECT
public:
    enum PartID {LPC2141 = 196353, LPC2142 = 196369, LPC2144 = 196370, LPC2146 = 196387, LPC2148 = 196389};
    enum Status {StatusNoError, StatusTimeOut, StatusError};

    explicit QLpcProg(QObject *parent = 0);
    virtual ~QLpcProg();
    void init(const QString &port = "");
    void deinit();
    void setCrystalValue(int value);
    void setBaudRate(int baudRate);
    void setEcho(bool echo = true);
    int readPartID();
    QString readBootCodeVersion();
    void unlock();

    void chipErase();
    void patchFirmware(QByteArray &data);
    void chipProgram(QByteArray chunk, int offset);

    Status getStatus();
    QString getStatusText();



    static QStringList detectSerialPorts();
    
signals:
    
public slots:

private:
    QList<QByteArray> encodeUU(const QByteArray &data);
    int encodeUUCheckSum(const QByteArray &data);
    void log_write(const QByteArray &data);

    QSerialPort m_port;
    Status m_status;
    QString m_statusText;
    bool m_EchoOn;

    
};

#endif // QLPCPROG_H
