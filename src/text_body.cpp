#include "text_body.h"

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

TextWorker::TextWorker(QObject *parent) : QObject(nullptr)
{
    localData = Shared::localData;

    connect(parent, &QObject::destroyed, this, &QObject::deleteLater);
    connect(this, &TextWorker::loop, this, &TextWorker::dumpTxtCache);
    loadSettings();
}

QString TextWorker::htmlEsc(const QString &txt)
{
    QString ret = txt;

    ret.replace(">", "&gt;");
    ret.replace("<", "&lt;");

    return ret;
}

void TextWorker::dumpTxtCache()
{
    QString txt;
    quint8  typeId;

    if (Shared::cacheTxt(Shared::TXT_OUT, typeId, txt))
    {
        *Shared::activeDisp = !Shared::cacheTxt(Shared::TXT_IS_EMPTY);

        if ((typeId == TEXT) || (typeId == PRIV_TEXT) | (typeId == PROMPT_TEXT))
        {
            addMainTxt(txt);

            if (typeId == PRIV_TEXT)
            {
                emit setUserIO(HIDDEN);
            }
        }
        else if (typeId == ERR)
        {
            addErrTxt(txt);
        }
        else if (typeId == BIG_TEXT)
        {
            addBigTxt(txt);
        }

        if (*Shared::activeDisp)
        {
            emit loop();
        }
    }
}

void TextWorker::toHtmlLines(const QString &txt, const QString &color)
{
    QStringList lines = htmlEsc(txt).split("\n");

    for (int i = 0; i < lines.size(); ++i)
    {
        emit htmlOut("<div style=\"white-space: pre; color:" + color + ";\">" + lines[i] + "</div>", i == lines.size() - 1);

        thread()->usleep(1000);
    }
}

void TextWorker::addMainTxt(const QString &txt)
{
    toHtmlLines(txt, mainColor);
}

void TextWorker::addErrTxt(const QString &txt)
{
    toHtmlLines(txt, errColor);
}

void TextWorker::addBigTxt(const QString &txt)
{
    QString     lines;
    QTextStream txtOut(&lines);

    wordWrap("", txtOut, txt, Shared::mainWidget);

    addMainTxt(lines);
}

void TextWorker::loadSettings()
{
    mainColor = localData->value("theme").toObject().value("text_color").toString();
    errColor  = localData->value("theme").toObject().value("err_color").toString();
}

TextBody::TextBody(QWidget *parent) : QTextEdit(parent)
{
    localData   = Shared::localData;
    txtDocument = new QTextDocument(this);
    txtCursor   = new QTextCursor(txtDocument);

    setDocument(txtDocument);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);
    loadTheme(localData, this);
    loadTextBodySettings();
}

void TextBody::loadTextBodySettings()
{
    txtDocument->setMaximumBlockCount(localData->value("max_lines").toInt());
}

void TextBody::setMaxLines(int value)
{
    txtDocument->setMaximumBlockCount(value);
}

void TextBody::reload()
{
    clear();

    loadTheme(localData, this);
    saveLocalData(localData);
    loadTextBodySettings();
}

void TextBody::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();

    menu->addAction("Clear", this, SLOT(clear()));
    menu->exec(event->globalPos());
    menu->deleteLater();
}

void TextBody::htmlIn(const QString &line, bool lastLine)
{
    moveCursor(QTextCursor::End);

    txtCursor->insertHtml(line);

    if (!lastLine)
    {
        txtCursor->insertBlock();
    }

    ensureCursorVisible();
}
