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
#include "cmd_objs/host_doc.h"
#include "main_ui.h"
#include "session.h"
#include "gen_file.h"
#include "text_body.h"
#include "cmd_line.h"

void setupClientCmds();
void setupCmdLine();
void setupGenFile();
void setupSession();
void setupText();
void setupProgBar();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);

    bool                     connected   = false;
    bool                     activeDisp  = false;
    ushort                   servMajor   = 0;
    ushort                   servMinor   = 0;
    ushort                   tcpRev      = 0;
    ushort                   modRev      = 0;
    quint16                  hostPort    = 0;
    quint16                  termCmdId   = 0;
    QByteArray               sessionId;
    QString                  clientHookCmd;
    QString                  hostAddress;
    QStringList              hookBypass;
    QHash<quint16, QString>  genfileCmds;
    QHash<quint16, quint8>   genfileTypes;
    QHash<QString, Command*> clientCmds;
    QHash<QString, Command*> hostDocs;
    QHash<quint16, QString>  hostCmds;
    QList<quint8>            idCache;
    QList<QString>           txtCache;
    QJsonObject              localData;

    loadLocalData(&localData);
    setupThemeSettings(&localData);

    Shared::connectedToHost = &connected;
    Shared::genfileCmds     = &genfileCmds;
    Shared::genfileTypes    = &genfileTypes;
    Shared::clientCmds      = &clientCmds;
    Shared::hostCmds        = &hostCmds;
    Shared::hostDocs        = &hostDocs;
    Shared::sessionId       = &sessionId;
    Shared::servMajor       = &servMajor;
    Shared::servMinor       = &servMinor;
    Shared::tcpRev          = &tcpRev;
    Shared::modRev          = &modRev;
    Shared::hostAddress     = &hostAddress;
    Shared::hostPort        = &hostPort;
    Shared::hookBypass      = &hookBypass;
    Shared::termCmdId       = &termCmdId;
    Shared::localData       = &localData;
    Shared::clientHookedCmd = &clientHookCmd;
    Shared::idCache         = &idCache;
    Shared::txtCache        = &txtCache;
    Shared::activeDisp      = &activeDisp;
    Shared::contextReloader = new ContextReloader(&app);
    Shared::theadKiller     = new ThreadKiller(&app);
    Shared::session         = new Session(&app);
    Shared::mainWin         = new MainWindow();
    Shared::textWorker      = new TextWorker(&app);
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
    new Halt(&app);
    new Resume(&app);
    new SetColors(&app);
    new SetFont(&app);
    new SetMaxLines(&app);
    new Genfile(&app);
    new ListCmds(&app);

    setupClientCmds();
    setupCmdLine();
    setupGenFile();
    setupSession();
    setupText();

    Shared::mainWin->setTextBody(Shared::textBody);
    Shared::mainWin->setCmdLine(Shared::cmdLine);
    Shared::mainWin->showUi();

    QDir::setCurrent(QDir::homePath());

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
    auto *session    = Shared::session;
    auto *cmdLine    = Shared::cmdLine;
    auto *textWorker = Shared::textWorker;
    auto *textBody   = Shared::textBody;
    auto *genFile    = Shared::genFile;
    auto *mainWin    = Shared::mainWin;
    auto *reloader   = Shared::contextReloader;
    auto *killer     = Shared::theadKiller;

    QObject::connect(mainWin, &MainWindow::closeApp, killer, &ThreadKiller::quitThreads);

    for (auto command : Shared::clientCmds->values())
    {
        QObject::connect(command, &Command::colorsChanged, textWorker, &TextWorker::loadSettings);
        QObject::connect(command, &Command::txtInCache, textWorker, &TextWorker::dumpTxtCache);

        QObject::connect(cmdLine, &CmdLine::dataToCommandObj, command, &Command::cmdCall);
        QObject::connect(cmdLine, &CmdLine::dataToHookedCmdObj, command, &Command::hookedCmdCall);

        QObject::connect(command, &Command::setUserIO, cmdLine, &CmdLine::setFlags);
        QObject::connect(command, &Command::unsetUserIO, cmdLine, &CmdLine::unsetFlags);

        QObject::connect(command, &Command::termHostCmd, session, &Session::termHostCmd);
        QObject::connect(command, &Command::haltHostcmd, session, &Session::yieldHostCmd);
        QObject::connect(command, &Command::resumeHostCmd, session, &Session::resumeHostCmd);
        QObject::connect(command, &Command::connectToHost, session, &Session::connectToServ);
        QObject::connect(command, &Command::quitApp, session, &Session::disconnectFromServ);
        QObject::connect(command, &Command::disconnectHost, session, &Session::disconnectFromServ);

        QObject::connect(command, &Command::quitApp, killer, &ThreadKiller::quitThreads);

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
    auto *session    = Shared::session;
    auto *cmdLine    = Shared::cmdLine;
    auto *textWorker = Shared::textWorker;
    auto *genFile    = Shared::genFile;

    QObject::connect(cmdLine, &CmdLine::dataToHost, session, &Session::binToServer);
    QObject::connect(cmdLine, &CmdLine::dataToHookedHost, session, &Session::hookedBinToServer);
    QObject::connect(cmdLine, &CmdLine::setHostCmdId, session, &Session::setCmdHook);

    QObject::connect(cmdLine, &CmdLine::dataToGenFile, genFile, &Genfile::dataIn);
    QObject::connect(cmdLine, &CmdLine::setGenfileType, genFile, &Genfile::setGenfileType);

    QObject::connect(cmdLine, &CmdLine::txtInCache, textWorker, &TextWorker::dumpTxtCache);
}

void setupGenFile()
{
    auto *session    = Shared::session;
    auto *cmdLine    = Shared::cmdLine;
    auto *textWorker = Shared::textWorker;
    auto *genFile    = Shared::genFile;
    auto *killer     = Shared::theadKiller;
    auto *genThr     = new QThread(QCoreApplication::instance());

    QObject::connect(genFile, &Genfile::termHostCmd, session, &Session::termHostCmd);
    QObject::connect(genFile, &Genfile::dataOut, session, &Session::hookedBinToServer);
    QObject::connect(genFile, &Genfile::enableGenFile, session, &Session::enableGenFile);
    QObject::connect(genFile, &Genfile::preCallTerm, session, &Session::idle);

    QObject::connect(genFile, &Genfile::setUserIO, cmdLine, &CmdLine::setFlags);
    QObject::connect(genFile, &Genfile::unsetUserIO, cmdLine, &CmdLine::unsetFlags);

    QObject::connect(genFile, &Genfile::txtInCache, textWorker, &TextWorker::dumpTxtCache);

    QObject::connect(genThr, &QThread::finished, killer, &ThreadKiller::threadFinished);
    QObject::connect(killer, &ThreadKiller::quitThreads, genThr, &QThread::quit);

    genFile->moveToThread(genThr);
    genThr->start();
}

void setupSession()
{
    auto *session    = Shared::session;
    auto *cmdLine    = Shared::cmdLine;
    auto *textWorker = Shared::textWorker;
    auto *genFile    = Shared::genFile;
    auto *killer     = Shared::theadKiller;
    auto *sesThr     = new QThread(QCoreApplication::instance());

    QObject::connect(session, &Session::hostFinished, genFile, &Genfile::finished);
    QObject::connect(session, &Session::toGenFile, genFile, &Genfile::dataIn);

    QObject::connect(session, &Session::setUserIO, cmdLine, &CmdLine::setFlags);
    QObject::connect(session, &Session::unsetUserIO, cmdLine, &CmdLine::unsetFlags);

    QObject::connect(session, &Session::txtInCache, textWorker, &TextWorker::dumpTxtCache);

    QObject::connect(sesThr, &QThread::finished, killer, &ThreadKiller::threadFinished);
    QObject::connect(killer, &ThreadKiller::quitThreads, sesThr, &QThread::quit);

    session->moveToThread(sesThr);
    sesThr->start();
}

void setupText()
{
    auto *textWorker = Shared::textWorker;
    auto *cmdLine    = Shared::cmdLine;
    auto *textBody   = Shared::textBody;
    auto *killer     = Shared::theadKiller;
    auto *txtThr     = new QThread(QCoreApplication::instance());

    QObject::connect(textWorker, &TextWorker::setUserIO, cmdLine, &CmdLine::setFlags);
    QObject::connect(textWorker, &TextWorker::unsetUserIO, cmdLine, &CmdLine::unsetFlags);

    QObject::connect(textWorker, &TextWorker::htmlOut, textBody, &TextBody::htmlIn);

    QObject::connect(txtThr, &QThread::finished, killer, &ThreadKiller::threadFinished);
    QObject::connect(killer, &ThreadKiller::quitThreads, txtThr, &QThread::quit);

    textWorker->moveToThread(txtThr);
    txtThr->start();
}

void setupProgBar()
{
    auto *session = Shared::session;
    auto *prog    = Shared::prog;

    QObject::connect(session, &Session::setProg, prog, &QProgressBar::setValue);
    QObject::connect(session, &Session::showProg, prog, &QProgressBar::setVisible);
}
