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
QString About::shortText() {return tr("display information about this MRCI client and display all available commands.");}
QString About::ioText()    {return tr("[{cmd_name}]/[text]");}
QString About::longText()  {return TXT_About;}

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

void About::listCmds(QHash<QString, Command *> *cmdObjs, QTextStream &txtOut, int largestCmd)
{
    QStringList cmdNames = cmdObjs->keys();

    cmdNames.sort(Qt::CaseInsensitive);

    for (int i = 0; i < cmdNames.size(); ++i)
    {
        wordWrap(cmdNames[i].leftJustified(largestCmd, ' ') + " - ", txtOut, cmdObjs->value(cmdNames[i])->shortText(), Shared::mainWidget);
    }
}

void About::dataIn(const QString &argsLine)
{
    QStringList args = parseArgs(argsLine);

    if (args.size() > 0)
    {
        QString cmdName = args[0].toLower().trimmed();

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
        txtOut << "usage: <command> <arguments>"                                              << endl << endl;
        txtOut << "<command>"                                                                 << endl << endl;

        QStringList cmdNames   = Shared::clientCmds->keys() + Shared::hostDocs->keys();
        int         largestCmd = 0;

        for (int i = 0; i < cmdNames.size(); ++i)
        {
            if (cmdNames[i].size() > largestCmd) largestCmd = cmdNames[i].size();
        }

        listCmds(Shared::clientCmds, txtOut, largestCmd);
        listCmds(Shared::hostDocs, txtOut, largestCmd);

        txtOut << endl << endl << "for more detailed information about a command type: about <command>" << endl << endl;

        cacheTxt(TEXT, txt);
    }
}

void About::run()
{
    dataIn(QString());
}
