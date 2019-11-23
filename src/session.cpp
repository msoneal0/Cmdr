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
    dSize     = 0;
    hook      = 0;
    dType     = 0;
    flags     = 0;
    reconnect = false;

    connect(parent, &QSslSocket::destroyed, this, &Session::deleteLater);

    connect(this, &Session::encrypted, this, &Session::handShakeDone);
    connect(this, &Session::connected, this, &Session::isConnected);
    connect(this, &Session::disconnected, this, &Session::isDisconnected);
    connect(this, &Session::readyRead, this, &Session::dataIn);

    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sockerr(QAbstractSocket::SocketError)));
}

void Session::hookedBinToServer(const QByteArray &data, quint8 typeId)
{
    if (*Shared::connectedToHost)
    {
        write(wrFrame(hook, data, typeId));
    }
    else
    {
        emit errTxtOut("err: not connected to a host.\n");
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
        emit errTxtOut("err: not connected to a host.\n");
    }
}

void Session::sockerr(QAbstractSocket::SocketError err)
{
    if (err == QAbstractSocket::RemoteHostClosedError)
    {
        emit mainTxtOut("\nThe remote host closed the session.\n");
    }
    else
    {
        emit errTxtOut("\nerr: " + errorString() + "\n");

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

    emit mainTxtOut("Connected.\n");

    QByteArray header;
    QString    appName = QString(APP_NAME) + " v" + QString(APP_VERSION);

    header.append(SERVER_HEADER_TAG);
    header.append(fixedToTEXT(appName, 134));
    header.append(fixedToTEXT(*Shared::hostAddress, 272));

    if (header.size() == CLIENT_HEADER_LEN)
    {
        write(header);
    }
    else
    {
        emit errTxtOut("\nerr: client bug! - header len not equal to " + QString::number(CLIENT_HEADER_LEN) + "\n");

        disconnectFromHost();
    }
}

void Session::handShakeDone()
{
    QSslCipher cipher = sessionCipher();
    QString    txt;

    QTextStream txtOut(&txt);

    txtOut << "SSL Handshake sucessful."                             << endl
           << "cipher details:"                                      << endl
           << " #cipher         - " << cipher.name()                 << endl
           << " #protocol       - " << cipher.protocolString()       << endl
           << " #bit_depth      - " << cipher.usedBits()             << endl
           << " #key_exchange   - " << cipher.keyExchangeMethod()    << endl
           << " #authentication - " << cipher.authenticationMethod() << endl;

    emit mainTxtOut(txt);
}

void Session::isDisconnected()
{
    dSize = 0;
    hook  = 0;
    dType = 0;
    flags = 0;

    *Shared::servMajor       = 0;
    *Shared::servMinor       = 0;
    *Shared::servPatch       = 0;
    *Shared::connectedToHost = false;

    Shared::sessionId->clear();
    Shared::hostCmds->clear();
    Shared::genfileCmds->clear();

    emit mainTxtOut("\nHost session ended. (disconnected)\n\n");

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
        emit mainTxtOut("Connecting to address: " + *Shared::hostAddress + " port: " + QString::number(*Shared::hostPort) + "\n");

        connectToHost(*Shared::hostAddress, *Shared::hostPort);
    }
}

void Session::disconnectFromServ()
{
    *Shared::connectedToHost = false;

    if (state() == QAbstractSocket::ConnectedState)
    {
        emit mainTxtOut("Disconnecting.\n");

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

void Session::haltHostCmd()
{
    if (hook != 0)
    {
        binToServer(hook, QByteArray(), HALT_CMD);
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
    if (dType == TEXT)
    {
        if (cmdId == ASYNC_RDY)
        {
            hook = 0;
        }

        emit mainTxtOut(fromTEXT(data));
    }
    else if (dType == BIG_TEXT)
    {
        emit bigTxtOut(fromTEXT(data));
    }
    else if (dType == ERR)
    {
        emit errTxtOut(fromTEXT(data));
    }
    else if (cmdId == ASYNC_SYS_MSG)
    {
        if (dType == HOST_CERT)
        {
            QSslCertificate cert(data, QSsl::Pem);
            QSslError       selfSigned(QSslError::SelfSignedCertificate, cert);

            emit mainTxtOut("SSL cert received.\n\n");

            if (cert.isSelfSigned())
            {
                emit mainTxtOut("WARNING: the cert is self signed. be careful if in a public network.\n\n");
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
            quint16 id = static_cast<quint16>(rdInt(data.mid(0, 2)));

            if (id == hook) hook = 0;

            emit hostCmdRemoved(id);
        }
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
        if ((dType == TEXT) || (dType == PRIV_TEXT))
        {
            emit mainTxtOut(fromTEXT(data));

            if (dType == PRIV_TEXT)
            {
                emit setUserIO(HIDDEN);
            }
        }
        else if (dType == BIG_TEXT)
        {
            emit bigTxtOut(fromTEXT(data));
        }
        else if (dType == ERR)
        {
            emit errTxtOut(fromTEXT(data));
        }
        else if (dType == IDLE)
        {
            hook = 0;

            emit hostFinished();
            emit unsetUserIO(HOST_HOOK);

            if (Shared::hostCmds->contains(cmdId))
            {
                emit mainTxtOut("\nFinished: " + Shared::hostCmds->value(cmdId) + "\n\n");
            }
            else
            {
                emit mainTxtOut("\nFinished: Unknown\n\n");
            }
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
            dataIn();
        }
    }
    else if (flags & VER_OK)
    {
        if (bytesAvailable() >= FRAME_HEADER_LEN)
        {
            QByteArray header = read(FRAME_HEADER_LEN);

            dType  = static_cast<quint8>(header[0]);
            cmdId  = static_cast<quint16>(rdInt(header.mid(1, 2)));
            branId = static_cast<quint16>(rdInt(header.mid(3, 2)));
            dSize  = static_cast<quint32>(rdInt(header.mid(5, 3)));
            flags |= DSIZE_RDY;

            dataIn();
        }
    }
    else if (bytesAvailable() >= SERVER_HEADER_LEN)
    {
        // host header format: [1byte(reply)][2bytes(major)][2bytes(minor)][2bytes(patch)][28bytes(sesId)]

        // reply is an 8bit little endian int the indicates the result of the client header
        // sent to the host from the isConnected() function.

        // sesId = 224bit sha3 hash
        // major = 16bit little endian uint (host ver major)
        // minor = 16bit little endian uint (host ver minor)
        // patch = 16bit little endian uint (host ver patch)

        quint8 reply = static_cast<quint8>(rdInt(read(1)));

        if ((reply == 1) || (reply == 2))
        {
            *Shared::servMajor = static_cast<quint16>(rdInt(read(2)));
            *Shared::servMinor = static_cast<quint16>(rdInt(read(2)));
            *Shared::servPatch = static_cast<quint16>(rdInt(read(2)));

            emit mainTxtOut("Detected host version: " + verText(*Shared::servMajor, *Shared::servMinor, *Shared::servPatch) + "\n");

            if (*Shared::servMajor == 2)
            {
                *Shared::sessionId       = read(28);
                *Shared::connectedToHost = true;

                flags |= VER_OK;

                emit mainTxtOut("Host assigned session id: " + Shared::sessionId->toHex() + "\n");

                if (reply == 2)
                {
                    emit mainTxtOut("Awaiting SSL cert from the host.\n");
                }
                else
                {
                    emit mainTxtOut("The host claims SSL is not needed. the connection will not be encrypted.\n");
                }
            }
            else
            {
                emit errTxtOut("err: Host not compatible.\n");

                disconnectFromHost();
            }
        }
        else if (reply == 4)
        {
            emit errTxtOut("err: The host was unable to find a SSL cert for common name: " + *Shared::hostAddress + ".\n");
        }
        else
        {
            emit errTxtOut("err: Invalid reply id from the host: " + QString::number(reply) + ".\n");
        }

        if ((reply != 1) && (reply != 2)) disconnectFromHost();

        dataIn();
    }
}
