#ifndef HOST_DOC_H
#define HOST_DOC_H

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

#include "command.h"

class HostDoc : public Command
{
    Q_OBJECT

private:

    QString shortTxt;
    QString ioTxt;
    QString longTxt;
    QString libTxt;
    quint16 cmdId;
    bool    valid;

    QString readNullTermText(const QByteArray &data, quint32 *offs);

public:

    QString shortText();
    QString ioText();
    QString longText();
    QString libText();

    explicit HostDoc(const QByteArray &import, QObject *parent = nullptr);

    bool isValid();

public slots:

    void cmdRemoved(quint16 id);
    void sessionEnded();
};

#endif // HOST_DOC_H
