#include "qlpcprog.h"

#include <QSerialPortInfo>
#include <QFile>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif

const char SYNCHRONIZED[] = "Synchronized\r\n";
const char SYNCHRONIZED_OK[] = "Synchronized\r\nOK\r\n";


QLpcProg::QLpcProg(QObject *parent) :
    QObject(parent),
    m_status(StatusNoError),
    m_EchoOn(true)
{
}

QLpcProg::~QLpcProg()
{
    deinit();
}

QStringList QLpcProg::detectSerialPorts()
{
    QStringList ret;

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    foreach(QSerialPortInfo port, ports)
    {
        ret.append(port.systemLocation());
    }

    return ret;
}

void QLpcProg::init(const QString &port)
{
    QByteArray recieved;

    deinit();

    m_port.setPortName(port);

    if (m_port.open(QIODevice::ReadWrite) == false)
    {
        m_status = StatusError;
        m_statusText = tr("Could't open serial port(%1)").arg(port);

        return;
    }

    m_port.setBaudRate(QSerialPort::Baud9600);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setFlowControl(QSerialPort::SoftwareControl);

    m_port.setDataTerminalReady(true); // RESET
    m_port.setRequestToSend(true);

#ifdef Q_OS_WIN
    Sleep(10);
#else
    usleep(10000);
#endif

    m_port.setDataTerminalReady(false); // RESET

#ifdef Q_OS_WIN
    Sleep(10);
#else
    usleep(10000);
#endif

    m_port.waitForReadyRead(100);
    m_port.readAll();

    m_port.write("?");
    log_write("?");

    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        if (recieved.count() >= (int)(sizeof(SYNCHRONIZED) - 1))
        {
            break;
        }
    }

    if (recieved != SYNCHRONIZED)
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved.");

        return;
    }

    m_port.write(SYNCHRONIZED);
    log_write(SYNCHRONIZED);
    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        if (recieved.count() >= (int)(sizeof(SYNCHRONIZED_OK) - 1))
        {
            break;
        }
    }

    if (recieved != SYNCHRONIZED_OK)
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved.");

        return;
    }

    m_status = StatusNoError;
    m_statusText.clear();
}

void QLpcProg::deinit()
{
    if (m_port.isOpen())
    {
        m_port.setDataTerminalReady(true); // RESET
        m_port.setRequestToSend(false);

        #ifdef Q_OS_WIN
            Sleep(10);
        #else
            usleep(10000);
        #endif

        m_port.setDataTerminalReady(false); // RESET

#ifdef Q_OS_WIN
    Sleep(10);
#else
    usleep(10000);
#endif

        m_port.close();
    }
}

void QLpcProg::setCrystalValue(int value)
{
    if (!m_port.isOpen())
    {
        m_status = StatusError;
        m_statusText = tr("Comm port is not open.");

        return;
    }

    QByteArray send = QByteArray::number(value).append("\r\n");
    QByteArray shouldRecieve;
    QByteArray recieved;

    if (m_EchoOn)
    {
        shouldRecieve = send.append("OK\r\n");
    }
    else
    {
        shouldRecieve = "OK\r\n";
    }

    m_port.write(send);
    log_write(send);
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        if (recieved.count() >= shouldRecieve.count())
        {
            break;
        }
    }

    if (recieved != shouldRecieve)
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved.");

        return;
    }

    m_status = StatusNoError;
    m_statusText.clear();
}

void QLpcProg::setBaudRate(int baudRate)
{
    if (!m_port.isOpen())
    {
        m_status = StatusError;
        m_statusText = tr("Comm port is not open.");

        return;
    }

    QByteArray recieved;
    QList<QByteArray> lines;
    QByteArray line;

    m_port.write("B " + QByteArray::number(baudRate) + " 1\r\n");
    log_write("B " + QByteArray::number(baudRate) + " 1\r\n");
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if (line != "0")
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }

    m_port.setBaudRate(baudRate);

    m_status = StatusNoError;
    m_statusText.clear();
}

void QLpcProg::setEcho(bool echo)
{
    if (!m_port.isOpen())
    {
        m_status = StatusError;
        m_statusText = tr("Comm port is not open.");

        return;
    }

    QByteArray recieved;
    QList<QByteArray> lines;
    QByteArray line;

    if (echo != m_EchoOn)
    {
        if (echo)
        {
            m_port.write("A 1\r\n");
            log_write("A 1\r\n");
        }
        else
        {
            m_port.write("A 0\r\n");
            log_write("A 0\r\n");
        }

        for(int c = 0; c < 1000; c++)
        {
            m_port.waitForReadyRead(1);
            recieved.append(m_port.readAll());

            lines = recieved.split('\n');

            if ((lines.count() == 4)&&(m_EchoOn == true))
            {
                line = lines.at(2);
                line.chop(1); // chop \r

                if (line == "0")
                {
                    m_EchoOn = !m_EchoOn;

                    m_status = StatusNoError;
                    m_statusText.clear();

                    return;
                }

                m_status = StatusError;
                m_statusText = tr("Wrong data recieved.");

                return;
            }

            if ((lines.count() == 2)&&(m_EchoOn == false))
            {
                line = lines.at(0);
                line.chop(1); // chop \r

                if (line == "0")
                {
                    m_EchoOn = !m_EchoOn;

                    m_status = StatusNoError;
                    m_statusText.clear();

                    return;
                }

                m_status = StatusError;
                m_statusText = tr("Wrong data recieved.");

                return;
            }
        }

        m_status = StatusTimeOut;
        m_statusText = tr("Data Timeout.");

        return;
    }

    m_status = StatusNoError;
    m_statusText.clear();
}

int QLpcProg::readPartID()
{
    if (!m_port.isOpen())
    {
        m_status = StatusError;
        m_statusText = tr("Comm port is not open.");

        return 0;
    }

    QByteArray send = "J\r\n";
    QByteArray recieved;
    QList<QByteArray> lines;
    QByteArray line;
    bool ok;
    int ret;

    m_port.write(send);
    log_write(send);
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        lines = recieved.split('\n');

        if ((lines.count() == 5)&&(m_EchoOn == true))
        {
            line = lines.at(3);
            line.chop(1); // chop \r

            ret = line.toInt(&ok) & 0x000FFFFF; // TODO: & is added so the code will work, nothing regarding it in the datasheet.

            if (!ok)
            {
                m_status = StatusTimeOut;
                m_statusText = tr("Data Timeout.");
            }
            else
            {
                m_status = StatusNoError;
                m_statusText.clear();
            }

            return ret;
        }

        if ((lines.count() == 3)&&(m_EchoOn == false))
        {
            line = lines.at(1);
            line.chop(1); // chop \r

            ret = line.toInt(&ok) & 0x000FFFFF; // TODO: & is added so the code will work, nothing regarding it in the datasheet.

            if (!ok)
            {
                m_status = StatusTimeOut;
                m_statusText = tr("Data Timeout.");
            }
            else
            {
                m_status = StatusNoError;
                m_statusText.clear();
            }

            return ret;
        }
    }

    m_status = StatusTimeOut;
    m_statusText = tr("Data Timeout.");

    return 0;
}

QString QLpcProg::readBootCodeVersion()
{
    if (!m_port.isOpen())
    {
        m_status = StatusError;
        m_statusText = tr("Comm port is not open.");

        return QString();
    }

    QByteArray send = "K\r\n";
    QByteArray recieved;

    m_port.write(send);
    log_write(send);
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() == 6)&&(m_EchoOn == true))
        {
            QString major = lines[4].left(lines[4].length() - 1);
            QString minor = lines[3].left(lines[3].length() - 1);

            return QString().append(major).append(".").append(minor);
        }
        if ((lines.count() == 4)&&(m_EchoOn == false)) // TODO: Test this branch.
        {
            QString major = lines[2].left(lines[2].length() - 1);
            QString minor = lines[1].left(lines[1].length() - 1);

            return QString().append(major).append(".").append(minor);
        }
    }

    m_status = StatusTimeOut;
    m_statusText = tr("Data Timeout.");

    return 0;
}

void QLpcProg::unlock()
{
    if (!m_port.isOpen())
    {
        m_status = StatusError;
        m_statusText = tr("Comm port is not open.");

        return;
    }

    QByteArray recieved;
    QByteArray line;

    m_port.write("U 23130\r\n");
    log_write("U 23130\r\n");
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if (line != "0")
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }
}

void QLpcProg::chipErase()
{
    if (!m_port.isOpen())
    {
        m_status = StatusError;
        m_statusText = tr("Comm port is not open.");

        return;
    }

    QByteArray recieved;
    QByteArray send;
    QByteArray line;

    switch(readPartID())
    {
    case LPC2141:
        send = "7";
        break;
    case LPC2142:
        send = "8";
        break;
    case LPC2144:
        send = "10";
        break;
    case LPC2146:
        send = "14";
        break;
    case LPC2148:
        send = "26";
        break;
    }

    if (!send.isEmpty())
    {
        unlock();
        if (m_status != StatusNoError)
        {
            return;
        }

        /// Prepare for erase
        m_port.write("P 0 " + send + "\r\n");
        log_write("P 0 " + send + "\r\n");
        for(int c = 0; c < 1000; c++)
        {
            m_port.waitForReadyRead(1);
            recieved.append(m_port.readAll());

            QList<QByteArray> lines = recieved.split('\n');

            if ((lines.count() >= 3)&&(m_EchoOn == true))
            {
                line = lines[1];
                line.chop(1);

                break;
            }
            if ((lines.count() >= 2)&&(m_EchoOn == false))
            {
                line = lines[0];
                line.chop(1);

                break;
            }
        }

        if (line != "0")
        {
            m_status = StatusError;
            m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

            return;
        }

        /// Erase
        m_port.write("E 0 " + send + "\r\n");
        log_write("E 0 " + send + "\r\n");
        for(int c = 0; c < 1000; c++)
        {
            m_port.waitForReadyRead(1);
            recieved.append(m_port.readAll());

            QList<QByteArray> lines = recieved.split('\n');

            if ((lines.count() >= 3)&&(m_EchoOn == true))
            {
                line = lines[1];
                line.chop(1);

                break;
            }
            if ((lines.count() >= 2)&&(m_EchoOn == false))
            {
                line = lines[0];
                line.chop(1);

                break;
            }
        }

        if (line != "0")
        {
            m_status = StatusError;
            m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

            return;
        }

        m_status = StatusNoError;
        m_statusText.clear();
    }
}

void QLpcProg::patchFirmware(QByteArray &data)
{
    quint32 *vectors = (quint32 *)data.data();
    quint32 signature = 0;

    for(int c = 0; c < 8; c++)
    {
        if (c != 5)
        {
            signature += vectors[c];
        }
    }

    vectors[5] = (quint32)0 - signature;
}

void QLpcProg::chipProgram(QByteArray chunk, int offset)
{
    if (!m_port.isOpen())
    {
        m_status = StatusError;
        m_statusText = tr("Comm port is not open.");

        return;
    }

    if (chunk.length() > 1024)
    {
        m_status = StatusError;
        m_statusText = tr("Programming buffer too big. Length is %1. It should be less or equal to 1024 bytes.").arg(QString(chunk.length()));

        return;
    }

    if (chunk.length() < 1024)
    {
        QByteArray new_chunk;
        new_chunk.resize(1024 - chunk.length());
        new_chunk.fill(255);

        chunk.append(new_chunk);
    }

    const QByteArray &chunk1 = chunk.left(512);
    const QByteArray &chunk2 = chunk.right(512);

    QList<QByteArray> encoded;
    QByteArray recieved;
    QByteArray line;

    encoded = encodeUU(chunk1);

    m_port.write("W 1073742336 512\r\n");
    log_write("W 1073742336 512\r\n");

    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if (line != "0")
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }

    foreach(QByteArray line, encoded)
    {
        m_port.write(line.append("\r\n"));
        log_write(line);
    }

    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if (line != "0")
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }

    m_port.write(QByteArray::number(encodeUUCheckSum(chunk1)).append("\r\n"));
    log_write(QByteArray::number(encodeUUCheckSum(chunk1)).append("\r\n"));

    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if ((line != "OK")&&(line != "0"))
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }

    encoded = encodeUU(chunk2);

    m_port.write("W 1073742848 512\r\n");
    log_write("W 1073742848 512\r\n");

    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if (line != "0")
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }

    foreach(QByteArray line, encoded)
    {
        m_port.write(line.append("\r\n"));
        log_write(line);
    }

    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if (line != "0")
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }

    m_port.write(QByteArray::number(encodeUUCheckSum(chunk2)).append("\r\n"));
    log_write(QByteArray::number(encodeUUCheckSum(chunk2)).append("\r\n"));

    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if ((line != "OK")&&(line != "0"))
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }

    m_port.write("P 0 26\r\n"); // TODO: Hardcoded 26(will only work with LPC2148)
    log_write("P 0 26\r\n"); // TODO: Hardcoded 26(will only work with LPC2148)

    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if (line != "0")
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }

    m_port.write("C " + QByteArray::number(offset) + " 1073742336 1024\r\n");
    log_write("C " + QByteArray::number(offset) + " 1073742336 1024\r\n");

    recieved.clear();
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        QList<QByteArray> lines = recieved.split('\n');

        if ((lines.count() >= 3)&&(m_EchoOn == true))
        {
            line = lines[1];
            line.chop(1);

            break;
        }
        if ((lines.count() >= 2)&&(m_EchoOn == false))
        {
            line = lines[0];
            line.chop(1);

            break;
        }
    }

    if (line != "0")
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }

    return;
}

QLpcProg::Status QLpcProg::getStatus()
{
    return m_status;
}

QString QLpcProg::getStatusText()
{
    return m_statusText;
}

QList<QByteArray> QLpcProg::encodeUU(const QByteArray &data)
{
    QList<QByteArray> ret;
    int chunks;

    const int CHUNK_SIZE = 45;


    chunks = data.length() / CHUNK_SIZE;
    if (data.length() % CHUNK_SIZE) chunks++;

    for(int c = 0; c < chunks; c++)
    {
        QByteArray chunk = data.mid(c * CHUNK_SIZE, CHUNK_SIZE);
        QByteArray line;

        line.append((char)(32 + chunk.length()));

        while(chunk.length() % 3 != 0)
        {
            if (chunk.length() == CHUNK_SIZE)
            {
                chunk.append('\0');
            }
            else
            {
                chunk.append(255);
            }
        }

        for(int group = 0; group < chunk.length() / 3; group++)
        {
            char tmp;

            tmp = (chunk.at((group * 3) + 0) >> 2) & 0x3F;
            if (tmp == 0x00) tmp = 0x60; else tmp += 0x20;
            line.append(tmp);

            tmp = ((chunk.at((group * 3) + 0) << 4) & 0x30)|((chunk.at((group * 3) + 1) >> 4) & 0x0F);
            if (tmp == 0x00) tmp = 0x60; else tmp += 0x20;
            line.append(tmp);

            tmp = ((chunk.at((group * 3) + 1) << 2) & 0x3C)|((chunk.at((group * 3) + 2) >> 6) & 0x03);
            if (tmp == 0x00) tmp = 0x60; else tmp += 0x20;
            line.append(tmp);

            tmp = chunk.at((group * 3) + 2) & 0x3F;
            if (tmp == 0x00) tmp = 0x60; else tmp += 0x20;
            line.append(tmp);
        }

        ret.append(line);
    }

    return ret;
}

int QLpcProg::encodeUUCheckSum(const QByteArray &data)
{
    int ret = 0;

    for (int c = 0; c < data.length(); c++)
    {
        ret += (unsigned char)data.at(c);
    }

    return ret;
}

void QLpcProg::log_write(const QByteArray &data)
{
    /*QFile log("write.log");

    log.open(QIODevice::Append);

    log.write(data);*/
}
