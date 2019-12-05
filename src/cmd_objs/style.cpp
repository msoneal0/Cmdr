#include "style.h"

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

void checkArgs(bool *changed, QJsonObject *jsonData, const QString &arg, const QStringList &args, const QString &dataKey, const QString &defaultData)
{
    if (argExists(arg, args))
    {
        QString     data = getParam(arg, args);
        QJsonObject obj  = jsonData->value("text_settings").toObject();

        if (data.isEmpty())
        {
            *changed = true;

            obj.insert(dataKey, defaultData);
        }
        else if (dataKey == "font_family")
        {
            *changed = true;

            obj.insert(dataKey, data);
        }
        else if (dataKey == "font_size")
        {
            bool ok;

            data.toInt(&ok);

            if (ok)
            {
                *changed = true;

                obj.insert(dataKey, data);
            }
        }
        else
        {
            QColor colorObj(data);

            if (colorObj.isValid())
            {
                *changed = true;

                obj.insert(dataKey, data);
            }
        }

        if (*changed)
        {
            jsonData->insert("text_settings", obj);
        }
    }
}

SetColors::SetColors(QObject *parent) : Command(parent)
{
    localData = Shared::localData;

    setObjectName("colors");

    Shared::clientCmds->insert(objectName(), this);
}

QString SetColors::shortText() {return tr("set the colors (text/background) of this terminal emulator.");}
QString SetColors::ioText()    {return tr("[{-text} ({color_code}) {-error} ({color_code}) {-bg} ({color_code})]/[none]");}
QString SetColors::longText()  {return TXT_SetColors;}

SetFont::SetFont(QObject *parent) : Command (parent)
{
    localData = Shared::localData;

    setObjectName("font");

    Shared::clientCmds->insert(objectName(), this);
}

QString SetFont::shortText() {return tr("set/view the text font family and size for this terminal emulator.");}
QString SetFont::ioText()    {return tr("[-name (text)]/[none]");}
QString SetFont::longText()  {return TXT_SetFont;}

SetMaxLines::SetMaxLines(QObject *parent) : Command (parent)
{
    localData = Shared::localData;

    setObjectName("max_lines");

    Shared::clientCmds->insert(objectName(), this);
}

QString SetMaxLines::shortText() {return tr("set/view the maximum amount of lines this terminal emulator can display.");}
QString SetMaxLines::ioText()    {return tr("[{-value (int)}]/[none]");}
QString SetMaxLines::longText()  {return TXT_SetMaxLines;}

void SetColors::dataIn(const QString &argsLine)
{
    bool        changed = false;
    QStringList args    = parseArgs(argsLine);

    checkArgs(&changed, localData, "-text",  args, "text_color", "#FBFBD7");
    checkArgs(&changed, localData, "-error", args, "err_color",  "#BB55D8");
    checkArgs(&changed, localData, "-bg",    args, "bg_color",   "#363A49");

    QJsonObject obj = localData->value("text_settings").toObject();

    cacheTxt(TEXT, "text:  " + obj.value("text_color").toString() + "\n");
    cacheTxt(TEXT, "error: " + obj.value("err_color").toString() + "\n");
    cacheTxt(TEXT, "bg:    " + obj.value("bg_color").toString() + "\n");

    if (changed) emit colorsChanged();
}

void SetFont::dataIn(const QString &argsLine)
{
    bool        changed = false;
    QStringList args    = parseArgs(argsLine);

    checkArgs(&changed, localData, "-name",  args, "font_family", "courier");
    checkArgs(&changed, localData, "-size", args, "font_size",  "9");

    QJsonObject obj = localData->value("text_settings").toObject();

    cacheTxt(TEXT, "family: " + obj.value("font_family").toString() + "\n");
    cacheTxt(TEXT, "size:   " + obj.value("font_size").toString() + "\n");

    if (changed) emit fontChanged();
}

void SetMaxLines::dataIn(const QString &argsLine)
{
    QStringList args   = parseArgs(argsLine);
    QString     valStr = getParam("-value", args);

    if (valStr.isEmpty())
    {
        cacheTxt(TEXT, QString::number(localData->value("max_lines").toInt()) + "\n");
    }
    else
    {
        bool ok;
        int  val = valStr.toInt(&ok);

        if (!ok)
        {
            cacheTxt(ERR, "err: '" + valStr + "' is not a valid integer.\n");
        }
        else if ((val < 50) || (val > 100000))
        {
            cacheTxt(ERR, "err: The value must range 50 - 100000.\n");
        }
        else
        {
            localData->insert("max_lines", val);

            saveLocalData(localData);

            emit setMaxLines(val);
        }
    }
}
