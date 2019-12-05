#ifndef GEN_FILE_H
#define GEN_FILE_H

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

#include "common.h"

class Genfile : public QObject
{
    Q_OBJECT

private:

    enum GenfileFlags
    {
        CLIENT_PARAMS_RDY = 1,
        HOST_PARAMS_RDY   = 1 << 1,
        SINGLE_STEP_MODE  = 1 << 2,
        TRUNCATE          = 1 << 3,
        DO_NOT_ASK        = 1 << 4,
        CONFIRM_NEEDED    = 1 << 5,
        WRITE_MODE        = 1 << 6,
        READ_MODE         = 1 << 7
    };

    uint    flags;
    QFile  *file;
    quint16 hook;
    qint64  bytesRW;
    QString localFile;
    QString remoteFile;
    QString len;
    QString offs;

    bool       wrToFile(const QByteArray &data);
    bool       rdFromFile();
    bool       seekToOffset();
    bool       lenOk();
    bool       atEnd();
    void       askOverwrite();
    void       setupForWriting();
    void       setupForReading();
    void       cacheTxt(quint8 typeId, QString txt);
    QByteArray autoFill(const QByteArray &data);
    qint64     getRdBuff();

public slots:

    void finished();
    void hookedDataIn(const QByteArray &data);
    void dataIn(quint16 cmdId, const QByteArray &data);

public:

    explicit Genfile(QObject *parent = nullptr);

signals:

    void txtInCache();
    void enableGenFile(bool state);
    void setUserIO(int flgs);
    void unsetUserIO(int flgs);
    void dataOut(quint16 cmdId, const QByteArray &data, uchar typeID = GEN_FILE);
    void hookedDataOut(const QByteArray &data, uchar typeID = GEN_FILE);
    void rdFileLoop(QByteArray data = QByteArray());
};

#endif // GEN_FILE_H
