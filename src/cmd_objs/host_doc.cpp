#include "host_doc.h"

//    This file is part of Cmdr.

//    Cmdr is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    Cmdr is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with Cmdr under the LICENSE.md file. If not, see
//    <http://www.gnu.org/licenses/>.

HostDoc::HostDoc(const QByteArray &import, QObject *parent) : Command(parent)
{
    valid = false;

    if (import.size() >= 259)
    {
        cmdId = static_cast<quint16>(rdInt(import.mid(0, 2)));

        QString cmdName = fromTEXT(import.mid(3, 128)).toLower();
        QString num;

        for (int i = 1; Shared::clientCmds->contains(cmdName + num); ++i)
        {
            num = "_" + QString::number(i);
        }

        setObjectName(cmdName + num);

        if (import[2] == 0x01)
        {
            Shared::genfileCmds->insert(cmdId, objectName());
        }

        quint32 offs = 259;

        libTxt   = fromTEXT(import.mid(131, 128));
        shortTxt = readNullTermText(import, &offs);
        ioTxt    = readNullTermText(import, &offs);
        longTxt  = readNullTermText(import, &offs);
        valid    = true;

        Shared::hostCmds->insert(cmdId, objectName());
        Shared::hostDocs->insert(objectName(), this);
    }
}

QString HostDoc::readNullTermText(const QByteArray &data, quint32 *offs)
{
    static const QByteArray null16(2, 0x00);

    QString ret;
    quint32 len = static_cast<quint32>(data.size());

    for (; *offs < len; *offs += 2)
    {
        if ((*offs + 2) < len)
        {
            QByteArray chr = QByteArray::fromRawData(data.data() + *offs, 2);

            if (chr == null16)
            {
                break;
            }
            else
            {
                ret.append(fromTEXT(chr));
            }
        }
    }

    *offs += 2;

    return ret;
}

QString HostDoc::shortText()
{
    return shortTxt;
}

QString HostDoc::ioText()
{
    return  ioTxt;
}
QString HostDoc::libText()
{
    return libTxt;
}

QString HostDoc::longText()
{
    return longTxt;
}

bool HostDoc::isValid()
{
    return valid;
}

void HostDoc::cmdRemoved(quint16 id)
{
    if (id == cmdId)
    {
        Shared::hostCmds->remove(cmdId);
        Shared::hostDocs->remove(objectName());
        Shared::genfileCmds->remove(cmdId);

        deleteLater();
    }
}

void HostDoc::sessionEnded()
{
    cmdRemoved(cmdId);
}
