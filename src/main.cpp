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

#include <QApplication>
#include <QThread>
#include <QHash>
#include <QTextStream>
#include <QJsonArray>

#include "cmd_objs/command.h"
#include "cmd_objs/status.h"
#include "cmd_objs/bookmarks.h"
#include "cmd_objs/info.h"
#include "cmd_objs/exec.h"
#include "cmd_objs/style.h"
#include "main_ui.h"
#include "session.h"
#include "gen_file.h"
#include "text_body.h"
#include "cmd_line.h"

void setupClientCmds();
void setupCmdLine();
void setupGenFile();
void setupSession();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);

    bool                     connected   = false;
    ushort                   servMajor   = 0;
    ushort                   servMinor   = 0;
    ushort                   servPatch   = 0;
    quint16                  hostPort    = 0;
    quint16                  termCmdId   = 0;
    QByteArray               sessionId;
    QString                  clientHookCmd;
    QString                  hostAddress;
    QStringList              hookBypass;
    QHash<quint16, QString>  genfileCmds;
    QHash<QString, Command*> clientCmds;
    QHash<quint16, QString>  hostCmds;
    QJsonObject              localData;

    loadLocalData(&localData);
    setupTextSettings(&localData);

    Shared::connectedToHost = &connected;
    Shared::genfileCmds     = &genfileCmds;
    Shared::clientCmds      = &clientCmds;
    Shared::hostCmds        = &hostCmds;
    Shared::sessionId       = &sessionId;
    Shared::servMajor       = &servMajor;
    Shared::servMinor       = &servMinor;
    Shared::servPatch       = &servPatch;
    Shared::hostAddress     = &hostAddress;
    Shared::hostPort        = &hostPort;
    Shared::hookBypass      = &hookBypass;
    Shared::termCmdId       = &termCmdId;
    Shared::localData       = &localData;
    Shared::clientHookedCmd = &clientHookCmd;
    Shared::contextReloader = new ContextReloader(&app);
    Shared::session         = new Session(&app);
    Shared::mainWin         = new MainWindow();
    Shared::textBody        = new TextBody(Shared::mainWin);
    Shared::cmdLine         = new CmdLine(Shared::mainWin);
    Shared::genFile         = new Genfile(&app);
    Shared::socket          = Shared::session;
    Shared::mainWidget      = Shared::mainWin;

    new SaveBookmark(&app);
    new ListBookmarks(&app);
    new DeleteBookmark(&app);
    new SeeBookmark(&app);
    new Status(&app);
    new Connect(&app);
    new Quit(&app);
    new About(&app);
    new EndSession(&app);
    new Term(&app);
    new SetColors(&app);
    new SetFont(&app);
    new SetMaxLines(&app);
    new Genfile(&app);

    setupClientCmds();
    setupCmdLine();
    setupGenFile();
    setupSession();

    Shared::mainWin->setTextBody(Shared::textBody);
    Shared::mainWin->setCmdLine(Shared::cmdLine);
    Shared::mainWin->showUi();

    return app.exec();
}

ContextReloader::ContextReloader(QObject *parent) : QObject(parent) {}

void ContextReloader::reloadCmdLine()
{
    Shared::cmdLine->close();

    Shared::cmdLine = new CmdLine(Shared::mainWin);

    Shared::mainWin->setCmdLine(Shared::cmdLine);

    for (auto command : Shared::clientCmds->values())
    {
        QObject::connect(Shared::cmdLine, &CmdLine::dataToCommandObj, command, &Command::cmdCall);
        QObject::connect(Shared::cmdLine, &CmdLine::dataToHookedCmdObj, command, &Command::hookedCmdCall);
    }

    setupCmdLine();
}

void setupClientCmds()
{
    Session         *session  = Shared::session;
    CmdLine         *cmdLine  = Shared::cmdLine;
    TextBody        *textBody = Shared::textBody;
    Genfile         *genFile  = Shared::genFile;
    MainWindow      *mainWin  = Shared::mainWin;
    ContextReloader *reloader = Shared::contextReloader;

    for (auto command : Shared::clientCmds->values())
    {
        QObject::connect(command, &Command::mainTxtOut, textBody, &TextBody::addMainTxt);
        QObject::connect(command, &Command::errTxtOut, textBody, &TextBody::addErrTxt);
        QObject::connect(command, &Command::bigTxtOut, textBody, &TextBody::addBigTxt);

        QObject::connect(cmdLine, &CmdLine::dataToCommandObj, command, &Command::cmdCall);
        QObject::connect(cmdLine, &CmdLine::dataToHookedCmdObj, command, &Command::hookedCmdCall);

        QObject::connect(command, &Command::setUserIO, cmdLine, &CmdLine::setFlags);
        QObject::connect(command, &Command::unsetUserIO, cmdLine, &CmdLine::unsetFlags);

        QObject::connect(command, &Command::termHostCmd, session, &Session::termHostCmd);
        QObject::connect(command, &Command::connectToHost, session, &Session::connectToServ);
        QObject::connect(command, &Command::quitApp, session, &Session::disconnectFromServ);
        QObject::connect(command, &Command::disconnectHost, session, &Session::disconnectFromServ);

        QObject::connect(command, &Command::quitApp, QCoreApplication::instance(), &QCoreApplication::quit);

        QObject::connect(command, &Command::termHostCmd, genFile, &Genfile::finished);
        QObject::connect(command, &Command::quitApp, genFile, &Genfile::finished);

        QObject::connect(command, &Command::colorsChanged, textBody, &TextBody::reload);
        QObject::connect(command, &Command::fontChanged, textBody, &TextBody::reload);
        QObject::connect(command, &Command::setMaxLines, textBody, &TextBody::setMaxLines);

        QObject::connect(command, &Command::colorsChanged, reloader, &ContextReloader::reloadCmdLine);
        QObject::connect(command, &Command::fontChanged, reloader, &ContextReloader::reloadCmdLine);

        QObject::connect(mainWin, &MainWindow::startup, command, &Command::onStartup);
    }
}

void setupCmdLine()
{
    Session  *session  = Shared::session;
    CmdLine  *cmdLine  = Shared::cmdLine;
    TextBody *textBody = Shared::textBody;
    Genfile  *genFile  = Shared::genFile;

    QObject::connect(cmdLine, &CmdLine::dataToHost, session, &Session::binToServer);
    QObject::connect(cmdLine, &CmdLine::dataToHookedHost, session, &Session::hookedBinToServer);

    QObject::connect(cmdLine, &CmdLine::mainTxtOut, textBody, &TextBody::addMainTxt);
    QObject::connect(cmdLine, &CmdLine::errTxtOut, textBody, &TextBody::addErrTxt);

    QObject::connect(cmdLine, &CmdLine::dataToGenFile, genFile, &Genfile::dataIn);
    QObject::connect(cmdLine, &CmdLine::dataToHookedGenFile, genFile, &Genfile::hookedDataIn);
}

void setupGenFile()
{
    Session  *session  = Shared::session;
    CmdLine  *cmdLine  = Shared::cmdLine;
    TextBody *textBody = Shared::textBody;
    Genfile  *genFile  = Shared::genFile;

    QObject::connect(genFile, &Genfile::dataOut, session, &Session::binToServer);
    QObject::connect(genFile, &Genfile::hookedDataOut, session, &Session::hookedBinToServer);
    QObject::connect(genFile, &Genfile::enableGenFile, session, &Session::enableGenFile);

    QObject::connect(genFile, &Genfile::setUserIO, cmdLine, &CmdLine::setFlags);
    QObject::connect(genFile, &Genfile::unsetUserIO, cmdLine, &CmdLine::unsetFlags);

    QObject::connect(genFile, &Genfile::mainTxtOut, textBody, &TextBody::addMainTxt);
    QObject::connect(genFile, &Genfile::errTxtOut, textBody, &TextBody::addErrTxt);
}

void setupSession()
{
    Session  *session  = Shared::session;
    CmdLine  *cmdLine  = Shared::cmdLine;
    TextBody *textBody = Shared::textBody;
    Genfile  *genFile  = Shared::genFile;

    QObject::connect(session, &Session::hostFinished, genFile, &Genfile::finished);
    QObject::connect(session, &Session::toGenFile, genFile, &Genfile::hookedDataIn);

    QObject::connect(session, &Session::setUserIO, cmdLine, &CmdLine::setFlags);
    QObject::connect(session, &Session::unsetUserIO, cmdLine, &CmdLine::unsetFlags);

    QObject::connect(session, &Session::bigTxtOut, textBody, &TextBody::addBigTxt);
    QObject::connect(session, &Session::mainTxtOut, textBody, &TextBody::addMainTxt);
    QObject::connect(session, &Session::errTxtOut, textBody, &TextBody::addErrTxt);
}
