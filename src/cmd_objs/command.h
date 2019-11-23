#ifndef COMMAND_H
#define COMMAND_H

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

#include "../common.h"

class Command : public QObject
{
    Q_OBJECT

private slots:

    void setHook(int flgs);
    void unsetHook(int flgs);

protected:

    bool activeHook();

public:

    virtual QString shortText();
    virtual QString ioText();
    virtual QString longText();
    virtual QString libText();
    virtual void    dataIn(const QString &) {}
    virtual void    term() {}

    explicit Command(QObject *parent = nullptr);

    void postExec();

public slots:

    virtual void onStartup() {}

    void cmdCall(const QString &cmdName, const QString &argsLine);
    void hookedCmdCall(const QString &argsLine);

signals:

    void mainTxtOut(const QString &txt);
    void errTxtOut(const QString &txt);
    void bigTxtOut(const QString &txt);
    void setUserIO(int flgs);
    void unsetUserIO(int flgs);
    void setMaxLines(int value);
    void termHostCmd();
    void haltHostcmd();
    void resumeHostCmd();
    void connectToHost();
    void quitApp();
    void disconnectHost();
    void colorsChanged();
    void fontChanged();
};

#endif // COMMAND_H
