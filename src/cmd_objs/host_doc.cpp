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
//    ```NEW_CMD```
//    This contains information about a new command that was added to the current session.

//    ```
//      format:
//      1. bytes[0-1]    2bytes   - 16bit LE unsigned int (command id)
//      2. bytes[2]      1byte    - 8bit LE unsigned int (genfile type)
//      3. bytes[3-66]   64bytes  - command name (TEXT - padded with 0x00)
//      4. bytes[67-130] 64bytes  - library name (TEXT - padded with 0x00)
//      5. bytes[131-n]  variable - short text (null terminated)
//      6. bytes[n-n]    variable - io text (null terminated)
//      7. bytes[n-n]    variable - long text (null terminated)

    valid = false;

    if (import.size() >= 131) // within safe import size
    {
        cmdId = static_cast<quint16>(rdInt(import.mid(0, 2))); // 1.

        auto cmdName = QString(import.mid(3, 64)).toLower(); // 3.

        QString num;

        for (int i = 1; Shared::clientCmds->contains(cmdName + num); ++i)
        {
            num = "_" + QString::number(i);
        }

        setObjectName(cmdName + num);

        if ((import[2] == GEN_UPLOAD) || (import[2] == GEN_DOWNLOAD)) // 2.
        {
            Shared::genfileCmds->insert(cmdId, objectName());
            Shared::genfileTypes->insert(cmdId, static_cast<quint8>(import[2]));
        }

        quint32 offs = 131; // 5.

        libTxt   = QString(import.mid(67, 64)); // 4.
        shortTxt = readNullTermText(import, &offs);
        ioTxt    = readNullTermText(import, &offs); // 6.
        longTxt  = readNullTermText(import, &offs); // 7.
        valid    = true;

        Shared::hostCmds->insert(cmdId, objectName());
        Shared::hostDocs->insert(objectName(), this);
    }
}

QString HostDoc::readNullTermText(const QByteArray &data, quint32 *offs)
{
    QString ret;

    auto len = static_cast<quint32>(data.size());

    for (; *offs < len; *offs += 1)
    {
        if (data[*offs] == 0x00)
        {
            break;
        }
        else
        {
            ret.append(data[*offs]);
        }
    }

    *offs += 1;

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
        Shared::genfileTypes->remove(cmdId);

        deleteLater();
    }
}

void HostDoc::sessionEnded()
{
    cmdRemoved(cmdId);
}
