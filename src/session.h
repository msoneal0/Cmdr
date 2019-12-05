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
#include "cmd_objs/host_doc.h"

#define SERVER_HEADER_TAG "MRCI"
#define CLIENT_HEADER_LEN 410
#define SERVER_HEADER_LEN 35
#define FRAME_HEADER_LEN  8

class Session : public QSslSocket
{
    Q_OBJECT

private:

    quint32 flags;
    quint32 dSize;
    quint16 cmdId;
    quint16 branId;
    quint16 hook;
    quint8  dType;
    bool    reconnect;

    void cacheTxt(quint8 typeId, QString txt);
    void dataFromHost(const QByteArray &data);
    void procAsync(const QByteArray &data);
    bool isAsync(quint16 id);

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

    void hookedBinToServer(const QByteArray &data, quint8 typeId);
    void binToServer(quint16 cmdId, const QByteArray &data, quint8 typeId);
    void enableGenFile(bool state);
    void termHostCmd();
    void haltHostCmd();
    void resumeHostCmd();
    void connectToServ();
    void disconnectFromServ();

signals:

    void hostFinished();
    void txtInCache();
    void loopDataIn();
    void setUserIO(int flgs);
    void unsetUserIO(int flgs);
    void hostCmdRemoved(quint16 id);
    void toGenFile(const QByteArray &data);
};

#endif // SOCKET_H
