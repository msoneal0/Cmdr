#include "status.h"

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

Status::Status(QObject *parent) : Command(parent)
{
    setObjectName("status");

    Shared::clientCmds->insert(objectName(), this);
}

QString Status::shortText() {return tr("view the current session parameters at the client's perspective.");}
QString Status::ioText()    {return tr("[none]/[text]");}
QString Status::longText()  {return TXT_SeeParams;}

void Status::dataIn(const QString &argsLine)
{
    Q_UNUSED(argsLine)

    QString     txt;
    QTextStream txtOut(&txt);

    txtOut << "--Local data" << endl << endl;
    txtOut << "  Client version : " << QCoreApplication::applicationVersion() << endl;
    txtOut << "  Connected?     : " << boolText(*Shared::connectedToHost)     << endl;
    txtOut << "  Address        : " << *Shared::hostAddress                   << endl;
    txtOut << "  Port           : " << *Shared::hostPort                      << endl;

    if (*Shared::connectedToHost)
    {
        txtOut << "" << endl;
        txtOut << "--Session data" << endl << endl;
        txtOut << "  Client address    : " << Shared::socket->localAddress().toString() << endl;
        txtOut << "  Host address      : " << Shared::socket->peerAddress().toString()  << endl;
        txtOut << "  Session id        : " << Shared::sessionId->toHex()                << endl;
        txtOut << "  Host version      : " << verText()                                 << endl;
        txtOut << "  GEN_FILE commands : ";

        QStringList genCmds = Shared::genfileCmds->values();

        if (genCmds.isEmpty())
        {
            genCmds.sort(Qt::CaseInsensitive);

            txtOut << "" << endl;
        }

        for (int i = 0; i < genCmds.size(); ++i)
        {
            if (i != 0) txtOut << "                      " << genCmds[i] << endl;
            else        txtOut << genCmds[i] << endl;
        }
    }

    cacheTxt(TEXT, txt);
}
