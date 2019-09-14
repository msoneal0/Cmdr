#ifndef BOOKMARKS_H
#define BOOKMARKS_H

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

#include <QFile>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>

#include "command.h"

class SaveBookmark : public Command
{
    Q_OBJECT

private:

    QString     baseName;
    QStringList baseArgs;

    void askOverwrite();

public:

    void run(const QString &name, QStringList &args);

    QString shortText();
    QString ioText();
    QString longText();
    void    term();

    explicit SaveBookmark(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//---------------------------------------

class ListBookmarks : public Command
{
    Q_OBJECT

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit ListBookmarks(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//--------------------------------------

class DeleteBookmark : public Command
{
    Q_OBJECT

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit DeleteBookmark(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//-------------------------------------

class SeeBookmark : public Command
{
    Q_OBJECT

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit SeeBookmark(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

#endif // BOOKMARKS_H
