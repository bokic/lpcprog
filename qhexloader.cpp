#include "qhexloader.h"
#include <QString>
#include <QFile>

QHexLoader::QHexLoader(const QString &p_Filename, QObject *parent)
    : QObject(parent)
{
    if (!p_Filename.isEmpty())
    {
        load(p_Filename);
    }
}

bool QHexLoader::load(const QString& p_Filename)
{
    QFile file(p_Filename);
    if (file.open(QIODevice::ReadOnly) == true)
    {
        m_Rows.clear();

		while(!file.atEnd())
        {
            QString l_Line = file.readLine(1024);

            if ((l_Line.endsWith("\r\n"))||(l_Line.endsWith("\n\r")))
                l_Line = l_Line.left(l_Line.length() - 2);
            else
                if ((l_Line.endsWith("\r"))||(l_Line.endsWith("\n")))
                    l_Line = l_Line.left(l_Line.length() - 1);

            if (l_Line.left(1) != ":")
                return false;

            if (l_Line.length() < 11)
                return false;

            if ((l_Line.length() % 2) == 0)
                return false;

            bool l_IsOK;

            int l_Size = l_Line.mid(1, 2).toInt(&l_IsOK, 16);
            if (l_IsOK == false)
                return false;

            if (l_Line.length() != 11 + (l_Size * 2))
                return false;

            QHexRow l_Row;

            l_Row.m_Address = l_Line.mid(3, 4).toInt(&l_IsOK, 16);
            if (l_IsOK == false)
                return false;

            l_Row.m_Type = l_Line.mid(7, 2).toInt(&l_IsOK, 16);
            if (l_IsOK == false)
                return false;

            for(int c = 0; c < l_Size; c++)
            {
                char ch;

                ch = l_Line.mid(9 + (c * 2), 2).toInt(&l_IsOK, 16);
                if (l_IsOK == false)
                    return false;

                l_Row.m_Data.append(ch);
            }


            quint8 l_checksum = 0;
            for(int c = 0; c < ((l_Line.length() - 3) / 2); c++)
            {
                l_checksum += l_Line.mid(1 + (c * 2), 2).toInt(&l_IsOK, 16);
                if (l_IsOK == false)
                    return false;
            }

            if (((0x100 - l_checksum) & 0xFF) != l_Line.mid(l_Line.length() - 2, 2).toInt(&l_IsOK, 16))
                return false;
            if (l_IsOK == false)
                return false;

            m_Rows.append(l_Row);
        }
    }
    else
        return false;

    if(m_Rows.length() == 0)
        return false;

    QHexRow l_LastRow = m_Rows[m_Rows.length() - 1];

    if ((l_LastRow.m_Address != 0x0000)||(l_LastRow.m_Data.count() != 0)||(l_LastRow.m_Type != 0x01))
        return false;

    return true;
}

QByteArray QHexLoader::data()
{
    QByteArray ret;

    quint16 page = 0;
    bool unknown = false;
    bool last = false;
    int pos;

    ret.reserve(512 * 1024); // Preallocate maximum size.

    foreach(QHexRow row, m_Rows)
    {
        switch(row.m_Type)
        {
        case 0:
            pos = (page * 0x10000) + row.m_Address;
            ret.resize(pos + row.m_Data.count());

            for(int c = 0; c < row.m_Data.count(); c++)
            {
                ret[pos + c] = row.m_Data.at(c);
            }
            break;
        case 1:
            last = true;
            break;
        case 4:
            if (row.m_Data.count() != 2)
            {
                return QByteArray();
            }

            page = (row.m_Data.at(0) << 8)|(row.m_Data.at(1) << 0);
            break;
        case 5:
            unknown = true; // TODO: Unknown hex line.
            break;
        default:
            return QByteArray();
        }
    }

    ret.squeeze(); // Reallocate down to needed size.

    return ret;
}
