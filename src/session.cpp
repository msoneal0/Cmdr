#include "session.h"

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

Session::Session(QObject *parent) : QSslSocket(nullptr)
{
    dSize          = 0;
    hook           = 0;
    dType          = 0;
    flags          = 0;
    progResetDelay = new QTimer(this);
    activeProg     = false;
    reconnect      = false;

    connect(parent, &QSslSocket::destroyed, this, &Session::deleteLater);
    connect(progResetDelay, &QTimer::timeout, this, &Session::resetProg);

    connect(this, &Session::encrypted, this, &Session::handShakeDone);
    connect(this, &Session::connected, this, &Session::isConnected);
    connect(this, &Session::disconnected, this, &Session::isDisconnected);
    connect(this, &Session::readyRead, this, &Session::dataIn);
    connect(this, &Session::loopDataIn, this, &Session::dataIn);

    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sockerr(QAbstractSocket::SocketError)));

    progResetDelay->setInterval(200);
    progResetDelay->setSingleShot(true);
}

void Session::hookedBinToServer(const QByteArray &data, quint8 typeId)
{
    if (*Shared::connectedToHost)
    {
        write(wrFrame(hook, data, typeId));
    }
    else
    {
        cacheTxt(ERR, "err: not connected to a host.\n");
    }
}

void Session::binToServer(quint16 cmdId, const QByteArray &data, quint8 typeId)
{
    if (*Shared::connectedToHost)
    {
        hook = cmdId;

        emit setUserIO(HOST_HOOK);

        write(wrFrame(cmdId, data, typeId));
    }
    else
    {
        cacheTxt(ERR, "err: not connected to a host.\n");
    }
}

void Session::setCmdHook(quint16 cmdId)
{
    hook = cmdId;
}

void Session::sockerr(QAbstractSocket::SocketError err)
{
    if (err == QAbstractSocket::RemoteHostClosedError)
    {
        cacheTxt(TEXT, "\nThe remote host closed the session.\n");
    }
    else
    {
        cacheTxt(ERR, "\nerr: " + errorString() + "\n");

        if (state() == QAbstractSocket::UnconnectedState)
        {
            hook = 0;
        }
    }
}

void Session::isConnected()
{
    // client header format: [4bytes(tag)][134bytes(appName)][272bytes(coName)]

    // tag     = 0x4D, 0x52, 0x43, 0x49 (MRCI)
    // appName = UTF16LE string (padded with 0x00)
    // coName  = UTF16LE string (padded with 0x00)

    // for MRCI host, if the client doesn't need to request a special SSL
    // cert from the host, it is good practice to fallback to using the
    // address used to connect to the host as the common name (coName).
    // in this case, it is used all of the time.

    cacheTxt(TEXT, "Connected.\n");

    auto appName = QString(APP_NAME) + " v" + QString(APP_VERSION);

    QByteArray header;

    header.append(SERVER_HEADER_TAG);
    header.append(fixedToTEXT(appName, 134));
    header.append(fixedToTEXT(*Shared::hostAddress, 272));

    if (header.size() == CLIENT_HEADER_LEN)
    {
        write(header);
    }
    else
    {
        cacheTxt(ERR, "\nerr: client bug! - header len not equal to " + QString::number(CLIENT_HEADER_LEN) + "\n");
        disconnectFromHost();
    }
}

void Session::handShakeDone()
{
    auto cipher = sessionCipher();

    QString     txt;
    QTextStream txtOut(&txt);

    txtOut << "SSL Handshake sucessful."                             << endl
           << "cipher details:"                                      << endl
           << " #cipher         - " << cipher.name()                 << endl
           << " #protocol       - " << cipher.protocolString()       << endl
           << " #bit_depth      - " << cipher.usedBits()             << endl
           << " #key_exchange   - " << cipher.keyExchangeMethod()    << endl
           << " #authentication - " << cipher.authenticationMethod() << endl;

    cacheTxt(TEXT, txt);
}

void Session::isDisconnected()
{
    dSize = 0;
    hook  = 0;
    dType = 0;
    flags = 0;

    *Shared::servMajor       = 0;
    *Shared::servMinor       = 0;
    *Shared::tcpRev          = 0;
    *Shared::modRev          = 0;
    *Shared::connectedToHost = false;

    Shared::sessionId->clear();
    Shared::hostCmds->clear();
    Shared::genfileCmds->clear();
    Shared::genfileTypes->clear();

    cacheTxt(TEXT, "\nHost session ended. (disconnected)\n\n");

    if (reconnect)
    {
        reconnect = false;

        connectToServ();
    }
}

void Session::connectToServ()
{
    if (state() == QAbstractSocket::ConnectedState)
    {
        reconnect = true;

        disconnectFromServ();
    }
    else
    {
        cacheTxt(TEXT, "Connecting to address: " + *Shared::hostAddress + " port: " + QString::number(*Shared::hostPort) + "\n");
        connectToHost(*Shared::hostAddress, *Shared::hostPort);
    }
}

void Session::disconnectFromServ()
{
    *Shared::connectedToHost = false;

    if (state() == QAbstractSocket::ConnectedState)
    {
        cacheTxt(TEXT, "Disconnecting.\n");
        disconnectFromHost();
    }

    close();
}

void Session::termHostCmd()
{
    if (hook != 0)
    {
        binToServer(hook, QByteArray(), TERM_CMD);
    }
}

void Session::yieldHostCmd()
{
    if (hook != 0)
    {
        binToServer(hook, QByteArray(), YIELD_CMD);
    }
}

void Session::resumeHostCmd()
{
    if (hook != 0)
    {
        binToServer(hook, QByteArray(), RESUME_CMD);
    }
}

void Session::enableGenFile(bool state)
{
    if (state)
    {
        flags |= GEN_FILE_ON;
    }
    else
    {
        flags &= ~GEN_FILE_ON;
    }
}

bool Session::isAsync(quint16 id)
{
    return id < 256;
}

void Session::procAsync(const QByteArray &data)
{
    if ((dType == TEXT) || (dType == BIG_TEXT) || (dType == ERR))
    {
        cacheTxt(dType, fromTEXT(data));

        if (cmdId == ASYNC_RDY)
        {
            hook = 0;
        }
    }
    else if (cmdId == ASYNC_SYS_MSG)
    {
        if (dType == HOST_CERT)
        {
            QSslCertificate cert(data, QSsl::Pem);
            QSslError       selfSigned(QSslError::SelfSignedCertificate, cert);

            cacheTxt(TEXT, "SSL cert received.\n\n");

            if (cert.isSelfSigned())
            {
                cacheTxt(TEXT, "WARNING: the host cert is self signed.\n\n");
            }

            ignoreSslErrors(QList<QSslError>() << selfSigned);
            addCaCertificate(cert);
            startClientEncryption();
        }
    }
    else if (cmdId == ASYNC_ADD_CMD)
    {
        if (dType == NEW_CMD)
        {
            auto *hostDoc = new HostDoc(data, this);

            if (hostDoc->isValid())
            {
                connect(this, &Session::disconnected, hostDoc, &HostDoc::sessionEnded);
                connect(this, &Session::hostCmdRemoved, hostDoc, &HostDoc::cmdRemoved);
            }
            else
            {
                hostDoc->deleteLater();
            }
        }
    }
    else if (cmdId == ASYNC_RM_CMD)
    {
        if ((dType == CMD_ID) && (data.size() == 2))
        {
            auto id = static_cast<quint16>(rdInt(data.mid(0, 2)));

            if (id == hook) hook = 0;

            emit hostCmdRemoved(id);
        }
    }
}

void Session::cacheTxt(quint8 typeId, QString txt)
{
    Shared::cacheTxt(Shared::TXT_IN, typeId, txt);

    if (!(*Shared::activeDisp))
    {
        emit txtInCache();
    }
}

void Session::idle()
{
    emit hostFinished();
    emit unsetUserIO(HOST_HOOK);

    if (Shared::hostCmds->contains(hook))
    {
        cacheTxt(TEXT, "\nFinished: " + Shared::hostCmds->value(hook) + "\n\n");
    }
    else
    {
        cacheTxt(TEXT, "\nFinished: Unknown\n\n");
    }

    hook = 0;
}

void Session::resetProg()
{
    emit setProg(0);
    emit showProg(false);

    activeProg = false;
}

void Session::startProg()
{
    emit showProg(true);

    activeProg = true;
}

void Session::updateProg(int value)
{
    emit setProg(value);

    if (!activeProg)
    {
        startProg();
    }
}

void Session::dataFromHost(const QByteArray &data)
{
    if (isAsync(cmdId))
    {
        procAsync(data);
    }
    else if ((cmdId == hook) && (branId == 0) && (hook != 0))
    {
        if ((dType == TEXT) || (dType == PRIV_TEXT) || (dType == BIG_TEXT) || (dType == ERR) || (dType == PROMPT_TEXT))
        {
            cacheTxt(dType, fromTEXT(data));
        }
        else if (dType == PROG)
        {
            updateProg(rdInt(data));
        }
        else if (dType == PROG_LAST)
        {
            progResetDelay->start();

            updateProg(rdInt(data));
        }
        else if (dType == IDLE)
        {
            if (activeProg && !progResetDelay->isActive())
            {
                resetProg();
            }

            idle();
        }
        else if ((dType == GEN_FILE) && (flags & GEN_FILE_ON))
        {
            emit toGenFile(data);
        }
    }
}

void Session::dataIn()
{
    if (flags & DSIZE_RDY)
    {
        if (bytesAvailable() >= dSize)
        {
            flags ^= DSIZE_RDY;

            dataFromHost(read(dSize));

            emit loopDataIn();
        }
    }
    else if (flags & VER_OK)
    {
        if (bytesAvailable() >= FRAME_HEADER_LEN)
        {
            auto header = read(FRAME_HEADER_LEN);

            dType  = static_cast<quint8>(header[0]);
            cmdId  = static_cast<quint16>(rdInt(header.mid(1, 2)));
            branId = static_cast<quint16>(rdInt(header.mid(3, 2)));
            dSize  = static_cast<quint32>(rdInt(header.mid(5, 3)));
            flags |= DSIZE_RDY;

            emit loopDataIn();
        }
    }
    else if (bytesAvailable() >= SERVER_HEADER_LEN)
    {
        // host header format: [1byte(reply)][2bytes(major)][2bytes(minor)][2bytes(tcp_rev)][2bytes(mod_rev)][28bytes(sesId)]

        // reply is an 8bit little endian int the indicates the result of the client header
        // sent to the host from the isConnected() function.

        // sesId = 224bit sha3 hash
        // major = 16bit little endian uint (host ver major)
        // minor = 16bit little endian uint (host ver minor)
        // patch = 16bit little endian uint (host ver patch)

        // reply 1: client header acceptable, SSL handshake not needed.
        // reply 2: client header acceptable, SSL handshake needed (STARTTLS).
        // reply 4: the common name sent by the client header was not found.

        auto reply = static_cast<quint8>(rdInt(read(1)));

        if ((reply == 1) || (reply == 2))
        {
            *Shared::servMajor = static_cast<quint16>(rdInt(read(2)));
            *Shared::servMinor = static_cast<quint16>(rdInt(read(2)));
            *Shared::tcpRev    = static_cast<quint16>(rdInt(read(2)));
            *Shared::modRev    = static_cast<quint16>(rdInt(read(2)));

            cacheTxt(TEXT, "Detected host version: " + verText() + "\n");

            if (*Shared::tcpRev == 0)
            {
                *Shared::sessionId       = read(28);
                *Shared::connectedToHost = true;

                flags |= VER_OK;

                cacheTxt(TEXT, "Host assigned session id: " + Shared::sessionId->toHex() + "\n");

                if (reply == 2)
                {
                    cacheTxt(TEXT, "Awaiting SSL cert from the host.\n");
                }
                else
                {
                    cacheTxt(TEXT, "The host claims SSL is not needed. the connection will not be encrypted.\n");
                }
            }
            else
            {
                cacheTxt(ERR, "err: Host not compatible.\n");
                disconnectFromHost();
            }
        }
        else if (reply == 4)
        {
            cacheTxt(ERR, "err: The host was unable to find a SSL cert for common name: " + *Shared::hostAddress + ".\n");
        }
        else
        {
            cacheTxt(ERR, "err: Invalid reply id from the host: " + QString::number(reply) + ".\n");
        }

        if ((reply != 1) && (reply != 2)) disconnectFromHost();

        emit loopDataIn();
    }
}
