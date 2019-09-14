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

TextBody::TextBody(QWidget *parent) : QTextEdit(parent)
{
    localData = Shared::localData;
    reloading = false;
    lineCount = 0;

    txtDocument = new QTextDocument(this);
    txtCursor   = new QTextCursor(txtDocument);

    setDocument(txtDocument);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);
    loadTextSettings(localData, this);
    loadTextBodySettings();

    txtDocument->setMaximumBlockCount(maxLines);
}

void TextBody::loadTextBodySettings()
{
    mainColor = localData->value("text_settings").toObject().value("text_color").toString();
    errColor  = localData->value("text_settings").toObject().value("err_color").toString();
    maxLines  = localData->value("max_lines").toInt();
}

QString TextBody::htmlEsc(const QString &txt)
{
    QString ret = txt;

    ret.replace(">", "&gt;");
    ret.replace("<", "&lt;");

    return ret;
}

void TextBody::addTextBlock(const QString &txt, const QString &color, TextType tType)
{
    QStringList lines = htmlEsc(txt).split("\n");

    lineCount += lines.size();

    for (int i = 0; i < lines.size(); ++i)
    {
        txtCursor->insertHtml("<div style=\"white-space: pre; color:" + color + ";\">" + lines[i] + "</div>");

        if (i == lines.size() - 1)
        {
            if (!reloading) textCache.append(QPair<TextType,QString>(tType, lines[i]));
        }
        else
        {
            if (!reloading) textCache.append(QPair<TextType,QString>(tType, lines[i] + "\n"));

            txtCursor->insertBlock();
        }

        ensureCursorVisible();
    }
}

void TextBody::setMaxLines(int value)
{
    txtDocument->setMaximumBlockCount(value);

    trimCache();
}

void TextBody::trimCache()
{
    for (;lineCount > maxLines; --lineCount)
    {
        textCache.removeFirst();
    }
}

void TextBody::addMainTxt(const QString &txt)
{
    moveCursor(QTextCursor::End);
    addTextBlock(txt, mainColor, MAIN);
    trimCache();
}

void TextBody::addErrTxt(const QString &txt)
{
    moveCursor(QTextCursor::End);
    addTextBlock(txt, errColor, ERROR);
    trimCache();
}

void TextBody::addBigTxt(const QString &txt)
{
    QString     lines;
    QTextStream txtOut(&lines);

    wordWrap("", txtOut, txt, Shared::mainWidget);

    addMainTxt(lines);
}

void TextBody::clearCache()
{
    lineCount = 0;

    textCache.clear();
}

void TextBody::reload()
{
    clear();

    loadTextSettings(localData, this);
    saveLocalData(localData);
    loadTextBodySettings();

    reloading = true;

    for (int i = 0; i < textCache.size(); ++i)
    {
        if (textCache[i].first == ERROR)
        {
            addErrTxt(textCache[i].second);
        }
        else
        {
            addMainTxt(textCache[i].second);
        }
    }

    reloading = false;
}

void TextBody::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu   *menu   = createStandardContextMenu();
    QAction *action = menu->addAction("Clear", this, SLOT(clear()));

    connect(action, SIGNAL(triggered()), this, SLOT(clearCache()));

    menu->exec(event->globalPos());
    menu->deleteLater();
}
