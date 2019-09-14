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

#include "cmd_objs/long_txt.h"

#define DEFAULT_HIST_LIMIT 100
#define DEFAULT_MAX_LINES  5000
#define RDBUFF             128000
#define CMD_ESCAPE         '#'
#define TXT_CODEC          "UTF-16LE"
#define BOOKMARK_FOLDER    "bookmarks"
#define CONFIG_FILENAME    "config.json"
#define APP_NAME           "Cmdr"
#define APP_TARGET         "cmdr"
#define APP_VERSION        "1.0.0"
#define MRCI_VERSION       "1.0.0"

enum TypeID
{
    GEN_FILE    = 30,
    TEXT        = 31,
    ERR         = 32,
    PRIV_TEXT   = 33,
    IDLE        = 34,
    HOST_CERT   = 35,
    FILE_INFO   = 36,
    PEER_INFO   = 37,
    MY_INFO     = 38,
    PEER_STAT   = 39,
    P2P_REQUEST = 40,
    P2P_CLOSE   = 41,
    P2P_OPEN    = 42,
    BYTES       = 43,
    SESSION_ID  = 44,
    NEW_CMD     = 45,
    CMD_ID      = 46,
    BIG_TEXT    = 47
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
class ContextReloader;

class Shared : public QObject
{
    Q_OBJECT

public:

    static bool                     *connectedToHost;
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
    static QJsonObject              *localData;
    static quint16                  *termCmdId;
    static CmdLine                  *cmdLine;
    static Genfile                  *genFile;
    static MainWindow               *mainWin;
    static Session                  *session;
    static TextBody                 *textBody;
    static ContextReloader          *contextReloader;
    static QWidget                  *mainWidget;

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


#endif // COMMON_H
