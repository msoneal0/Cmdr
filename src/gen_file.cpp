#include "gen_file.h"

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

Genfile::Genfile(QObject *parent) : QObject(nullptr)
{
    file = new QFile(this);

    finished();

    connect(parent, &QObject::destroyed, this, &QObject::deleteLater);
    connect(this, &Genfile::rdFileLoop, this, &Genfile::dataIn);
}

void Genfile::cacheTxt(quint8 typeId, QString txt)
{
    Shared::cacheTxt(Shared::TXT_IN, typeId, txt);

    if (!(*Shared::activeDisp))
    {
        emit txtInCache();
    }
}

void Genfile::finished()
{
    if (flags & ACTIVE)
    {
        flags   = 0;
        bytesRW = 0;

        localFile.clear();
        remoteFile.clear();
        len.clear();
        offs.clear();

        if (file->isOpen())
        {
            file->close();
        }

        emit unsetUserIO(GEN_HOOK);
        emit enableGenFile(false);
        emit termHostCmd();
    }
}

void Genfile::askOverwrite()
{
    flags |= CONFIRM_NEEDED;

    emit setUserIO(GEN_HOOK);
    emit cacheTxt(TEXT, "About to overwrite file: '" + localFile + "' do you want to continue? (y/n): ");
}

bool Genfile::seekToOffset()
{
    bool ret = false;

    if (offs.isEmpty())
    {
        ret = file->seek(0);
    }
    else
    {
        bool   ok;
        qint64 pos = offs.toLongLong(&ok, 10);

        if (!ok)
        {
            cacheTxt(ERR, "err: An invalid offset was provided: " + offs + "\n");
        }

        ret = file->seek(pos);
    }

    return ret;
}

bool Genfile::lenOk()
{
    bool ret = false;

    if (len.isEmpty() && (flags & READ_MODE))
    {
        len = QString::number(file->bytesAvailable());
        ret = true;
    }
    else if (len.isEmpty())
    {
        cacheTxt(ERR, "err: The -len parameter is empty.\n");
    }
    else
    {
        len.toLongLong(&ret, 10);
    }

    return ret;
}

void Genfile::setupForWriting()
{
    flags |= WRITE_MODE;

    QFile::OpenMode mode = QFile::ReadWrite;

    if (flags & TRUNCATE)
    {
        mode |= QFile::Truncate;
    }

    file->setFileName(localFile);

    if (!file->open(mode))
    {
        cacheTxt(ERR, "err: Could not open the client file for writing. reason: " + file->errorString() + "\n");
        finished();
    }
    else if (!seekToOffset())
    {
        cacheTxt(ERR, "err: Could not seek to offset postion: " + offs + " of the client file.\n");
        finished();
    }
    else if (!lenOk())
    {
        cacheTxt(ERR, "err: The -len parameter (" + len + ") is invalid.\n");
        finished();
    }
    else
    {
        emit dataOut(QByteArray());
    }
}

void Genfile::setupForReading()
{
    flags |= READ_MODE;

    file->setFileName(localFile);

    if (!file->open(QFile::ReadOnly))
    {
        cacheTxt(ERR, "err: Could not open the client file for reading. reason: " + file->errorString() + "\n");
        finished();
    }
    else if (!seekToOffset())
    {
        cacheTxt(ERR, "err: Could not seek to offset postion: " + offs + " of the client file.\n");
        finished();
    }
    else if (!lenOk())
    {
        cacheTxt(ERR, "err: The -len parameter (" + len + ") is invalid.\n");
        finished();
    }
}

bool Genfile::atEnd()
{
    return (bytesRW >= len.toLongLong()) || ((flags & READ_MODE) && file->atEnd());
}

bool Genfile::wrToFile(const QByteArray &data)
{
    bool   ret   = false;
    qint64 bytes = file->write(data);

    if (bytes != -1)
    {
        ret      = true;
        bytesRW += bytes;

        if (atEnd())
        {
            finished();
        }
        else if (flags & SINGLE_STEP_MODE)
        {
            emit dataOut(QByteArray());
        }
    }
    else
    {
        cacheTxt(ERR, "err: Client write failure - " + file->errorString() + "\n");
    }

    return ret;
}

qint64 Genfile::getRdBuff()
{
    qint64 buff           = RDBUFF;
    qint64 bytesAvailable = len.toLongLong() - bytesRW;

    if (buff > bytesAvailable)
    {
        buff = bytesAvailable;
    }

    return buff;
}

bool Genfile::rdFromFile()
{
    bool       ret = false;
    QByteArray data;

    data.resize(static_cast<int>(getRdBuff()));

    if (file->read(data.data(), data.size()) != -1)
    {
        ret      = true;
        bytesRW += data.size();

        emit dataOut(data);

        if (atEnd())
        {
            finished();
        }
        else if (!(flags & SINGLE_STEP_MODE))
        {
            emit rdFileLoop();
        }
    }
    else
    {
        cacheTxt(ERR, "err: Client read failure - " + file->errorString() + "\n");
    }

    return ret;
}

void Genfile::setGenfileType(quint8 typeId)
{
    if (typeId == GEN_UPLOAD)
    {
        flags |= READ_MODE;
    }
    else if (typeId == GEN_DOWNLOAD)
    {
        flags |= WRITE_MODE;
    }
}

QByteArray Genfile::autoFill(const QByteArray &data)
{
    auto args = parseArgs(data, -1);
    auto ind  = args.indexOf(QRegularExpression("-len", QRegularExpression::CaseInsensitiveOption));

    if (ind == -1)
    {
        len = QString::number(QFileInfo(localFile).size());

        args.append("-len");
        args.append(len);

        return args.join(' ').toUtf8();
    }
    else
    {
        return data;
    }
}

void Genfile::dataIn(const QByteArray &data)
{
    if (!(flags & CLIENT_PARAMS_RDY))
    {
        QStringList args = parseArgs(data, -1);

        localFile  = getParam("-client_file", args);
        remoteFile = getParam("-remote_file", args);
        len        = getParam("-len", args);
        offs       = getParam("-offset", args);

        if (args.contains("-force", Qt::CaseInsensitive))
        {
            flags |= DO_NOT_ASK;
        }

        flags |= CLIENT_PARAMS_RDY;
        flags |= ACTIVE;

        emit enableGenFile(true);

        if (flags & READ_MODE)
        {
            if (!argExists("-client_file", args))
            {
                cacheTxt(ERR, "err: The client file (-client_file) argument was not found.\n");
                finished();

                emit preCallTerm();
            }
            else if (!QFile::exists(localFile))
            {
                cacheTxt(ERR, "err: The client file: '" + localFile + "' does not exists.\n");
                finished();

                emit preCallTerm();
            }
            else
            {
                emit setUserIO(HOST_HOOK);
                emit dataOut(autoFill(data));
            }
        }
        else
        {
            emit setUserIO(HOST_HOOK);
            emit dataOut(data);
        }
    }
    else if (!(flags & HOST_PARAMS_RDY))
    {
        QStringList args = parseArgs(data, -1);

        if (args.contains("-single_step", Qt::CaseInsensitive))
        {
            flags |= SINGLE_STEP_MODE;
        }

        if (flags & READ_MODE)
        {
            setupForReading();

            flags |= HOST_PARAMS_RDY;
        }
        else if (flags & WRITE_MODE)
        {
            if (args.contains("-truncate", Qt::CaseInsensitive))
            {
                flags |= TRUNCATE;
            }

            if (args.contains("-len", Qt::CaseInsensitive))
            {
                len = getParam("-len", args);
            }

            if (QFile::exists(localFile) && !(flags & DO_NOT_ASK))
            {
                askOverwrite();
            }
            else
            {
                setupForWriting();
            }

            flags |= HOST_PARAMS_RDY;
        }
        else
        {
            cacheTxt(ERR, "err: The client doesn't know if this command call is an upload or download. this is a bug, please report it.\n");
            finished();
        }
    }
    else if (flags & CONFIRM_NEEDED)
    {
        auto ans = QString(data);

        if ("y" == ans.toLower())
        {
            emit unsetUserIO(GEN_HOOK);

            flags &= ~CONFIRM_NEEDED;

            setupForWriting();
        }
        else if ("n" == ans.toLower())
        {
            finished();
        }
        else
        {
            askOverwrite();
        }
    }
    else if (flags & WRITE_MODE)
    {
        if (!wrToFile(data)) finished();
    }
    else if (flags & READ_MODE)
    {
        if (!rdFromFile()) finished();
    }
}
