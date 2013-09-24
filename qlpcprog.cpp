#include "qlpcprog.h"

#include <qserialportinfo.h>
#include <QDateTime>
#include <QFile>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif

static const char SYNCHRONIZED[] = "Synchronized";
static const char SYNCHRONIZED_CRNL[] = "Synchronized\r\n";
static const char SYNCHRONIZED_OK[] = "Synchronized\r\nOK\r\n";

#define PORT_OPEN_CHECK(ret) \
    if (!m_port.isOpen()) \
    { \
        m_status = StatusError; \
        m_statusText = tr("Comm port is not open."); \
      \
        return ret; \
    }


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

void QLpcProg::sendRecieve(const QByteArray &send, int lines, const QByteArray shouldRecieve)
{
    QByteArray recieved;

    if (lines < 1)
    {
        m_status = StatusError;
        m_statusText = tr("Invalid function parametar.");

        return;
    }

    m_port.write(send);
    log_write("SEND - " + send);
    for(int c = 0; c < 1000; c++)
    {
        m_port.waitForReadyRead(1);
        recieved.append(m_port.readAll());

        if (recieved.endsWith("\r\n"))
        {
            QList<QByteArray> recieved_lines = recieved.split('\n');

            if (recieved_lines.count() <= lines)
            {
                continue;
            }
            else if (recieved_lines.count() > lines + 1)
            {
                m_status = StatusError;
                m_statusText = tr("Too much data returned.");

                log_write("RECIEVE - " + recieved);

                return;
            }
            else
            {
                QByteArray ret = recieved_lines.at(lines - 1);

                ret = ret.left(ret.length() - 1); // remove \r

                log_write("RECIEVE - " + recieved);

                if (ret != shouldRecieve)
                {
                    m_status = StatusError;
                    m_statusText = tr("Too much data returned.");

                }
                else
                {
                    m_status = StatusNoError;
                    m_statusText.clear();
                }

                return;
            }

            break;
        }
    }

    m_status = StatusTimeOut;
    m_statusText.clear();

    log_write("RECIEVE - " + recieved);
}

void QLpcProg::init(const QString &port)
{
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
    m_port.setStopBits(QSerialPort::TwoStop);
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

    sendRecieve("?", 1, SYNCHRONIZED);
    if (m_status != StatusNoError) return;


    sendRecieve(SYNCHRONIZED_CRNL, 2, "OK");
    if (m_status != StatusNoError) return;
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
    QByteArray command;

    PORT_OPEN_CHECK();

    command = QByteArray::number(value) + "\r\n";

    if (m_EchoOn)
    {
        sendRecieve(command, 2, "OK");
    }
    else
    {
        sendRecieve(command, 1, "OK");
    }
}

void QLpcProg::setBaudRate(int baudRate)
{
    QByteArray command;

    PORT_OPEN_CHECK();

    if (m_EchoOn)
    {
        sendRecieve(command, 2, "0");
    }
    else
    {
        sendRecieve(command, 1, "0");
    }

    if (m_status != StatusNoError) return;

    if (m_port.setBaudRate(baudRate) == false)
    {
        m_status = StatusError;
        m_statusText = tr("Can\'t change baudrate. Internal error(%1).").arg(QString(m_port.errorString()));

        return;
    }
}

void QLpcProg::setEcho(bool echo)
{
    PORT_OPEN_CHECK();

    if (echo != m_EchoOn)
    {
        QByteArray command;

        if (echo)
        {
            command = "A 1\r\n";
        }
        else
        {
            command = "A 0\r\n";
        }

        sendRecieve(command, 1, "0");

        m_EchoOn = echo;

        return;
    }

    m_status = StatusNoError;
    m_statusText.clear();
}

int QLpcProg::readPartID()
{
    PORT_OPEN_CHECK(0);

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
    PORT_OPEN_CHECK(QString());

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
    PORT_OPEN_CHECK();

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

    if ((line != "OK")&&(line != "0"))
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return;
    }
}

void QLpcProg::chipErase()
{
    PORT_OPEN_CHECK();

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
    default:
        // TODO:
        return;
    }

    /// Unlock commands
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

bool QLpcProg::chipBlankCheck()
{
    PORT_OPEN_CHECK(false);

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
    default:
        // TODO:
        return false;
    }

    // Blank check
    m_port.write("I 1 " + send + "\r\n"); // Skip first sector according UM10139 chapter 21.8.10
    log_write("I 1 " + send + "\r\n");

    m_port.waitForBytesWritten(-1);

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

    if ((line != "0")&&(line != "8"))
    {
        m_status = StatusError;
        m_statusText = tr("Wrong data recieved(%1).").arg(QString(line));

        return false;
    }

    m_status = StatusNoError;
    m_statusText.clear();

    if (line == "8")
    {
        return false;
    }

    // Now check the first sector.

    return true;
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
    PORT_OPEN_CHECK();

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

   m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.write(QByteArray::number(encodeUUCheckSum(chunk1)).append("\r\n"));
    log_write(QByteArray::number(encodeUUCheckSum(chunk1)).append("\r\n"));

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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
        m_port.waitForBytesWritten(-1);
        log_write(line);
    }

    m_port.write(QByteArray::number(encodeUUCheckSum(chunk2)).append("\r\n"));
    log_write(QByteArray::number(encodeUUCheckSum(chunk2)).append("\r\n"));

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.write("C " + QByteArray::number(offset) + " 1073742336 1024\r\n");
    log_write("C " + QByteArray::number(offset) + " 1073742336 1024\r\n");

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

void QLpcProg::chipVerify(QByteArray chunk, int offset)
{
    PORT_OPEN_CHECK();

    int orig_size = chunk.length();

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

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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

    m_port.write(QString("M " + QByteArray::number(offset) + " 1073742336 " + QString::number(orig_size) + "\r\n").toLatin1());
    log_write(QString("M " + QByteArray::number(offset) + " 1073742336 " + QString::number(orig_size) + "\r\n").toLatin1());

    m_port.waitForBytesWritten(-1);

    recieved.clear();
    line.clear();
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
#ifdef QT_NO_DEBUG
    Q_UNUSED(data);
#else
    QFile log("write.log");

    log.open(QIODevice::Append);

    log.write(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toUtf8() + " - " + data);

    if (!data.endsWith("\r\n"))
    {
        log.write("\r\n");
    }

#endif
}
