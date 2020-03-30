#include "info.h"

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

About::About(QObject *parent) : Command(parent)
{
    setObjectName("about");

    Shared::clientCmds->insert(objectName(), this);
}

void    About::onStartup() {dataIn(QString());}
QString About::shortText() {return tr("display information about the client or a command.");}
QString About::ioText()    {return tr("[{cmd_name}]/[text]");}
QString About::longText()  {return TXT_About;}

ListCmds::ListCmds(QObject *parent) : Command(parent)
{
    setObjectName("ls_cmds");

    Shared::clientCmds->insert(objectName(), this);
}

QString ListCmds::shortText() {return tr("display all available commands.");}
QString ListCmds::ioText()    {return tr("[none]/[text]");}
QString ListCmds::longText()  {return TXT_ListCmds;}

void About::dispInfo(Command *cmdObj)
{
    QString     txt;
    QTextStream txtOut(&txt);

    wordWrap("i/o:     ", txtOut, cmdObj->ioText(), Shared::mainWidget);
    txtOut << "" << endl;
    wordWrap("library: ", txtOut, cmdObj->libText(), Shared::mainWidget);
    txtOut << "" << endl;
    wordWrap("usage:   ", txtOut, cmdObj->longText(), Shared::mainWidget);

    cacheTxt(TEXT, txt);
}

bool About::dispClientCmd(const QString &cmdName)
{
    bool ret = false;

    if (Shared::clientCmds->contains(cmdName))
    {
        dispInfo(Shared::clientCmds->value(cmdName));

        ret = true;
    }

    return ret;
}

bool About::dispHostCmd(const QString &cmdName)
{
    bool ret = false;

    if (Shared::hostDocs->contains(cmdName))
    {
        dispInfo(Shared::hostDocs->value(cmdName));

        ret = true;
    }

    return ret;
}

bool About::dispInfo(const QString &cmdName)
{
    if (dispClientCmd(cmdName))
    {
        return true;
    }
    else if (dispHostCmd(cmdName))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void About::dataIn(const QString &argsLine)
{
    auto args = parseArgs(argsLine);

    if (args.size() > 0)
    {
        auto cmdName = args[0].toLower().trimmed();

        if (!dispInfo(cmdName))
        {
            cacheTxt(ERR, "err: No such command: '" + cmdName + "'\n");
        }
    }
    else
    {
        QString     txt;
        QTextStream txtOut(&txt);

        txtOut << libText()                                                                   << endl << endl;
        txtOut << "Based on QT " << QT_VERSION_STR << " " << 8 * QT_POINTER_SIZE << "bit"     << endl << endl;
        txtOut << "The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE" << endl;
        txtOut << "WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE." << endl << endl;
        txtOut << "run: 'ls_cmds' to see all available commands."                             << endl << endl;
        txtOut << "for more detailed information about a command run: 'about <command>'"      << endl << endl;

        cacheTxt(TEXT, txt);
    }
}

void About::run()
{
    dataIn(QString());
}

void ListCmds::ls(QHash<QString, Command *> *cmdObjs, QTextStream &txtOut, const QString &title)
{
    if (!cmdObjs->isEmpty())
    {
        auto cmdNames   = cmdObjs->keys();
        auto largestCmd = 0;

        for (int i = 0; i < cmdNames.size(); ++i)
        {
            if (cmdNames[i].size() > largestCmd) largestCmd = cmdNames[i].size();
        }

        cmdNames.sort(Qt::CaseInsensitive);

        txtOut << endl;
        txtOut << title << endl << endl;

        for (int i = 0; i < cmdNames.size(); ++i)
        {
            wordWrap(cmdNames[i].leftJustified(largestCmd, ' ') + " - ", txtOut, cmdObjs->value(cmdNames[i])->shortText(), Shared::mainWidget);
        }
    }
}

void ListCmds::dataIn(const QString &argsLine)
{
    Q_UNUSED(argsLine)

    QString     txt;
    QTextStream txtOut(&txt);

    ls(Shared::clientCmds, txtOut, "Client Commands:");
    ls(Shared::hostDocs, txtOut, "Host Commands:");

    cacheTxt(TEXT, txt);
}
