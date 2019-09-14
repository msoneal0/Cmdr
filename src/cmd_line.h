#ifndef CMDLINE_H
#define CMDLINE_H

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

class CmdLine : public QComboBox
{
    Q_OBJECT

private:

    QJsonObject *localData;
    QString      hiddenText;
    int          flags;

    void    keyReleaseEvent(QKeyEvent *event);
    void    contextMenuEvent(QContextMenuEvent *event);
    void    procCmdLine(const QString &line);
    void    echo(const QString &line);
    void    syncHistToFile();
    void    syncHistFromFile();
    void    duplicateScan(const QString &txt);
    void    toHost(const QString &cmdName, const QString &args);
    void    toLocalCmd(const QString &cmdName, const QString &args);
    QString cmdNameOnly(const QString &line);

private slots:

    void selectionCheck();
    void hideText(const QString &txt);
    void checkForHistReset(const QString &txt);

public:

    explicit CmdLine(QWidget *parent = nullptr);

public slots:

    void setFlags(int flgs);
    void unsetFlags(int flgs);
    void clearHist();

signals:

    void dataToCommandObj(const QString &cmdName, const QString &argsLine);
    void dataToHookedCmdObj(const QString &argsLine);
    void dataToHost(quint16 cmdId, const QByteArray &data, uchar dType);
    void dataToHookedHost(const QByteArray &data, uchar dType);
    void dataToGenFile(quint16 cmdId, const QByteArray &data);
    void dataToHookedGenFile(const QByteArray &data);
    void mainTxtOut(const QString &txt);
    void errTxtOut(const QString &txt);
};

#endif // CMDLINE_H
