#ifndef COMMON_H
#define COMMON_H

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

#include <QObject>
#include <QLineEdit>
#include <QTextEdit>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QDockWidget>
#include <QSize>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QGuiApplication>
#include <QScreen>
#include <QResizeEvent>
#include <QPalette>
#include <QKeyEvent>
#include <QTextOption>
#include <QTextCursor>
#include <QTextStream>
#include <QThread>
#include <QInputMethodEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPaintEvent>
#include <QAction>
#include <QPixmapCache>
#include <QPair>
#include <QTextDocument>
#include <QBrush>
#include <QColor>
#include <QTextBlockFormat>
#include <QComboBox>
#include <QDebug>
#include <QTextCodec>
#include <QCoreApplication>
#include <QStringList>
#include <QHostAddress>
#include <QtEndian>
#include <QDir>
#include <QJsonDocument>
#include <QSslSocket>
#include <QFile>
#include <QFile>
#include <QRegExp>
#include <QFileInfo>
#include <QMutex>

#include "cmd_objs/long_txt.h"

#define DEFAULT_HIST_LIMIT 100
#define DEFAULT_MAX_LINES  1000
#define RDBUFF             128000
#define TXT_CODEC          "UTF-16LE"
#define BOOKMARK_FOLDER    "bookmarks"
#define CONFIG_FILENAME    "config.json"
#define APP_NAME           "Cmdr"
#define APP_TARGET         "cmdr"
#define APP_VERSION        "2.1.0"

enum TypeID : quint8
{
    GEN_FILE              = 1,
    TEXT                  = 2,
    ERR                   = 3,
    PRIV_TEXT             = 4,
    IDLE                  = 5,
    HOST_CERT             = 6,
    FILE_INFO             = 7,
    PEER_INFO             = 8,
    MY_INFO               = 9,
    PEER_STAT             = 10,
    P2P_REQUEST           = 11,
    P2P_CLOSE             = 12,
    P2P_OPEN              = 13,
    BYTES                 = 14,
    SESSION_ID            = 15,
    NEW_CMD               = 16,
    CMD_ID                = 17,
    BIG_TEXT              = 18,
    TERM_CMD              = 19,
    HOST_VER              = 20,
    PRIV_IPC              = 21,
    PUB_IPC               = 22,
    PUB_IPC_WITH_FEEDBACK = 23,
    PING_PEERS            = 24,
    CH_MEMBER_INFO        = 25,
    CH_ID                 = 26,
    KILL_CMD              = 27,
    HALT_CMD              = 28,
    RESUME_CMD            = 29
};

enum AsyncCommands : quint16
{
    ASYNC_RDY               = 1,   // client   | none
    ASYNC_SYS_MSG           = 2,   // client   | none
    ASYNC_CAST              = 4,   // client   | public
    ASYNC_USER_DELETED      = 7,   // client   | public
    ASYNC_TO_PEER           = 16,  // client   | public  | retricted
    ASYNC_LIMITED_CAST      = 17,  // client   | public
    ASYNC_P2P               = 19,  // client   | public
    ASYNC_NEW_CH_MEMBER     = 21,  // client   | public
    ASYNC_DEL_CH            = 22,  // client   | public
    ASYNC_RENAME_CH         = 23,  // client   | public
    ASYNC_NEW_SUB_CH        = 25,  // client   | public
    ASYNC_RM_SUB_CH         = 26,  // client   | public
    ASYNC_RENAME_SUB_CH     = 27,  // client   | public
    ASYNC_INVITED_TO_CH     = 28,  // client   | public
    ASYNC_RM_CH_MEMBER      = 29,  // client   | public
    ASYNC_INVITE_ACCEPTED   = 30,  // client   | public
    ASYNC_MEM_LEVEL_CHANGED = 31,  // client   | public
    ASYNC_SUB_CH_LEVEL_CHG  = 32,  // client   | public
    ASYNC_ADD_RDONLY        = 33,  // client   | public
    ASYNC_RM_RDONLY         = 34,  // client   | public
    ASYNC_ADD_CMD           = 35,  // client   | none
    ASYNC_RM_CMD            = 36,  // client   | none
};

enum ChannelMemberLevel
{
    OWNER   = 1,
    ADMIN   = 2,
    OFFICER = 3,
    REGULAR = 4,
    PUBLIC  = 5
};

enum UserIOFlags
{
    HOST_HOOK  = 1,
    LOCAL_HOOK = 1 << 2,
    GEN_HOOK   = 1 << 3,
    HIDDEN     = 1 << 4
};

void        setupTextSettings(QJsonObject *data);
void        loadTextSettings(QJsonObject *data, QWidget *widget);
void        saveLocalData(QJsonObject *obj);
void        loadLocalData(QJsonObject *obj);
void        wordWrap(const QString &label, QTextStream &txtOut, const QString &txtIn, QWidget *measureWid);
bool        argExists(const QString &key, const QStringList &args);
QByteArray  wrInt(quint64 num, int numOfBits);
QByteArray  wrFrame(quint16 cmdId, const QByteArray &data, uchar dType);
QByteArray  toTEXT(const QString &txt);
QByteArray  fixedToTEXT(const QString &txt, int len);
QStringList parseArgs(const QByteArray &data, int maxArgs);
QStringList parseArgs(const QString &line);
QString     fromTEXT(const QByteArray &txt);
QString     appDataDir();
QString     getParam(const QString &key, const QStringList &args);
QString     extractCmdName(const QByteArray &data);
QString     boolText(bool state);
QString     verText(quint16 maj, quint16 min, quint16 patch);
quint64     rdInt(const QByteArray &bytes);

class Command;
class CmdLine;
class Genfile;
class MainWindow;
class Session;
class TextBody;
class TextWorker;
class ContextReloader;
class HostDoc;
class ThreadKiller;

class Shared : public QObject
{
    Q_OBJECT

public:

    static bool                     *connectedToHost;
    static bool                     *activeDisp;
    static QByteArray               *sessionId;
    static ushort                   *servMajor;
    static ushort                   *servMinor;
    static ushort                   *servPatch;
    static QString                  *hostAddress;
    static QString                  *clientHookedCmd;
    static quint16                  *hostPort;
    static QStringList              *hookBypass;
    static QSslSocket               *socket;
    static QHash<QString, Command*> *clientCmds;
    static QHash<quint16, QString>  *hostCmds;
    static QHash<quint16, QString>  *genfileCmds;
    static QHash<QString, Command*> *hostDocs;
    static QList<quint8>            *idCache;
    static QList<QString>           *txtCache;
    static QJsonObject              *localData;
    static quint16                  *termCmdId;
    static CmdLine                  *cmdLine;
    static Genfile                  *genFile;
    static MainWindow               *mainWin;
    static Session                  *session;
    static TextBody                 *textBody;
    static TextWorker               *textWorker;
    static ContextReloader          *contextReloader;
    static QWidget                  *mainWidget;
    static ThreadKiller             *theadKiller;

    enum CacheOp
    {
        TXT_IN,
        TXT_OUT,
        TXT_CLEAR,
        TXT_IS_EMPTY
    };

    static bool cacheTxt(CacheOp op);
    static bool cacheTxt(CacheOp op, quint8 &typeId, QString &txt);

    explicit Shared(QObject *parent = nullptr);
};

class ContextReloader : public QObject
{
    Q_OBJECT

public:

    explicit ContextReloader(QObject *parent = nullptr);

public slots:

    void reloadCmdLine();
};

class ThreadKiller : public QObject
{
    Q_OBJECT

private:

    int threadCount;

public:

    explicit ThreadKiller(QObject *parent = nullptr);

public slots:

    void threadFinished();

signals:

    void quitThreads();
};


#endif // COMMON_H
