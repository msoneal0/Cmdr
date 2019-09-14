#ifndef STYLE_H
#define STYLE_H

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

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QColor>

#include "command.h"

void checkArgs(bool *changed, QJsonObject *jsonData, const QString &arg, const QStringList &args, const QString &dataKey, const QString &defaultData);

class SetColors: public Command
{
    Q_OBJECT

private:

    QJsonObject *localData;

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit SetColors(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//-------------------------

class SetFont: public Command
{
    Q_OBJECT

private:

    QJsonObject *localData;

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit SetFont(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

//--------------------------

class SetMaxLines: public Command
{
    Q_OBJECT

private:

    QJsonObject *localData;

public:

    QString shortText();
    QString ioText();
    QString longText();

    explicit SetMaxLines(QObject *parent = nullptr);

public slots:

    void dataIn(const QString &argsLine);
};

#endif // STYLE_H
