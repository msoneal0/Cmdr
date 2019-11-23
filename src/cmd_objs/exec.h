#ifndef EXEC_H
#define EXEC_H

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

#include <QTimer>

#include "bookmarks.h"
#include "command.h"

class Connect : public Command
{
    Q_OBJECT

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit Connect(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//------------------------------------------

class Quit : public Command
{
    Q_OBJECT

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit Quit(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//----------------------------------------

class EndSession : public Command
{
    Q_OBJECT

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit EndSession(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//---------------------------------------

class Term : public Command
{
    Q_OBJECT

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit Term(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//----------------------------------------

class Halt : public Command
{
    Q_OBJECT

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit Halt(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//----------------------------------------

class Resume : public Command
{
    Q_OBJECT

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit Resume(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

#endif // EXEC_H
