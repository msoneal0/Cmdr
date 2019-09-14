#ifndef SOCKET_H
#define SOCKET_H

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

#include <openssl/ssl.h>
#include <openssl/x509.h>

#include <QObject>
#include <QSslSocket>
#include <QHostAddress>
#include <QFile>
#include <QAbstractSocket>
#include <QCoreApplication>
#include <QTextCodec>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QSsl>
#include <QSslKey>
#include <QSslCipher>
#include <QSslCertificate>
#include <QSocketNotifier>

#include "cmd_objs/command.h"
#include "cmd_objs/bookmarks.h"

#define SERVER_HEADER_TAG "MRCI"
#define CLIENT_HEADER_LEN 410
#define SERVER_HEADER_LEN 35
#define FRAME_HEADER_LEN  6

#define ASYNC_RDY               1
#define ASYNC_SYS_MSG           2
#define ASYNC_EXE_CRASH         3
#define ASYNC_TO_PEER           21
#define ASYNC_LIMITED_CAST      22
#define ASYNC_P2P               24
#define ASYNC_NEW_CH_MEMBER     26
#define ASYNC_DEL_CH            27
#define ASYNC_RENAME_CH         28
#define ASYNC_CH_ACT_FLAG       29
#define ASYNC_NEW_SUB_CH        30
#define ASYNC_RM_SUB_CH         31
#define ASYNC_RENAME_SUB_CH     32
#define ASYNC_INVITED_TO_CH     33
#define ASYNC_RM_CH_MEMBER      34
#define ASYNC_INVITE_ACCEPTED   35
#define ASYNC_MEM_LEVEL_CHANGED 36
#define ASYNC_SUB_CH_LEVEL_CHG  37
#define ASYNC_ADD_RDONLY        38
#define ASYNC_RM_RDONLY         39
#define ASYNC_ADD_CMD           40
#define ASYNC_RM_CMD            41
#define ASYNC_USER_RENAMED      42

class Session : public QSslSocket
{
    Q_OBJECT

private:

    uint    flags;
    uint    dSize;
    quint16 cmdId;
    quint16 hook;
    uchar   dType;
    bool    reconnect;

    void dataFromHost(const QByteArray &data);

private slots:

    void dataIn();
    void isConnected();
    void isDisconnected();
    void handShakeDone();
    void sockerr(QAbstractSocket::SocketError err);

public:

    enum SessionFlags : uint
    {
        VER_OK      = 1,
        DSIZE_RDY   = 1 << 2,
        GEN_FILE_ON = 1 << 3
    };

    explicit Session(QObject *parent = nullptr);

public slots:

    void hookedBinToServer(const QByteArray &data, uchar typeId);
    void binToServer(quint16 cmdId, const QByteArray &data, uchar typeId);
    void enableGenFile(bool state);
    void termHostCmd();
    void connectToServ();
    void disconnectFromServ();

signals:

    void hostFinished();
    void setUserIO(int flgs);
    void unsetUserIO(int flgs);
    void toGenFile(const QByteArray &data);
    void mainTxtOut(const QString &txt);
    void errTxtOut(const QString &txt);
    void bigTxtOut(const QString &txt);
};

#endif // SOCKET_H
