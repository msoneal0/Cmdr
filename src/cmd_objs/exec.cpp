#include "exec.h"

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

Connect::Connect(QObject *parent) : Command(parent)
{
    setObjectName("connect");

    Shared::clientCmds->insert(objectName(), this);
}

QString Connect::shortText() {return tr("connect a MRCI host.");}
QString Connect::ioText()    {return tr("[-addr (text) -port (text) {-save (bookmark)}] or [-load (bookmark)]/[text]");}
QString Connect::longText()  {return TXT_Connect;}

Quit::Quit(QObject *parent) : Command(parent)
{
    setObjectName("quit");

    Shared::hookBypass->append(objectName());
    Shared::clientCmds->insert(objectName(), this);
}

QString Quit::shortText() {return tr("close the client application.");}
QString Quit::ioText()    {return tr("[none]/[none]");}
QString Quit::longText()  {return TXT_Quit;}

EndSession::EndSession(QObject *parent) : Command(parent)
{
    setObjectName("end_session");

    Shared::hookBypass->append(objectName());
    Shared::clientCmds->insert(objectName(), this);
}

QString EndSession::shortText() {return tr("disconnect from the host.");}
QString EndSession::ioText()    {return tr("[none]/[none]");}
QString EndSession::longText()  {return TXT_EndSession;}

Term::Term(QObject *parent) : Command(parent)
{
    setObjectName("term");

    Shared::hookBypass->append(objectName());
    Shared::clientCmds->insert(objectName(), this);
}

QString Term::shortText() {return tr("terminate the currently running host command.");}
QString Term::ioText()    {return tr("[none]/[CMD_ID]");}
QString Term::longText()  {return TXT_Term;}

Halt::Halt(QObject *parent) : Command(parent)
{
    setObjectName("yield");

    Shared::hookBypass->append(objectName());
    Shared::clientCmds->insert(objectName(), this);
}

QString Halt::shortText() {return tr("yield/pause the currently running host command.");}
QString Halt::ioText()    {return tr("[none]/[CMD_ID]");}
QString Halt::longText()  {return TXT_Term;}

Resume::Resume(QObject *parent) : Command(parent)
{
    setObjectName("resume");

    Shared::hookBypass->append(objectName());
    Shared::clientCmds->insert(objectName(), this);
}

QString Resume::shortText() {return tr("resume the currently halted/paused host command.");}
QString Resume::ioText()    {return tr("[none]/[CMD_ID]");}
QString Resume::longText()  {return TXT_Term;}

void Connect::dataIn(const QString &argsLine)
{
    QStringList args = parseArgs(argsLine);

    *Shared::hostAddress = getParam("-addr", args);
    *Shared::hostPort    = getParam("-port", args).toUShort();

    QString saveName = getParam("-save", args);
    QString loadName = getParam("-load", args);

    if (!saveName.isEmpty())
    {
        SaveBookmark(this).run(saveName, args);
    }

    if (!loadName.isEmpty())
    {
        QFile file(appDataDir() + BOOKMARK_FOLDER + "/" + loadName + ".json", this);

        if (file.open(QFile::ReadOnly))
        {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

            *Shared::hostAddress = doc.object().value("address").toString();
            *Shared::hostPort    = static_cast<quint16>(doc.object().value("port").toInt());
        }
        else
        {
            cacheTxt(ERR, "err: Could not open the requested bookmark for reading, reason: " + file.errorString() + "\n");
        }

        file.close();
    }

    if (Shared::hostAddress->isEmpty())
    {
        cacheTxt(ERR, "err: Host address is empty.\n");
    }
    else if (QHostAddress(*Shared::hostAddress).isNull())
    {
        cacheTxt(ERR, "err: '" + *Shared::hostAddress + "' is not a valid address.\n");
    }
    else if (*Shared::hostPort == 0)
    {
        cacheTxt(ERR, "err: The host port cannot be 0.\n");
    }
    else
    {
        emit connectToHost();
    }
}

void Quit::dataIn(const QString &argsLine)
{
    Q_UNUSED(argsLine)

    emit quitApp();
}

void EndSession::dataIn(const QString &argsLine)
{
    Q_UNUSED(argsLine)

    if (*Shared::connectedToHost)
    {
        emit disconnectHost();
    }
}

void Term::dataIn(const QString &argsLine)
{
    Q_UNUSED(argsLine)

    Shared::cacheTxt(Shared::TXT_CLEAR);

    emit termHostCmd();
}

void Halt::dataIn(const QString &argsLine)
{
    Q_UNUSED(argsLine)

    emit haltHostcmd();
}

void Resume::dataIn(const QString &argsLine)
{
    Q_UNUSED(argsLine)

    emit resumeHostCmd();
}
