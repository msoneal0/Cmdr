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
    localData   = Shared::localData;
    txtDocument = new QTextDocument(this);
    txtCursor   = new QTextCursor(txtDocument);

    setDocument(txtDocument);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);
    loadTextSettings(localData, this);
    loadTextBodySettings();
}

void TextBody::loadTextBodySettings()
{
    mainColor = localData->value("text_settings").toObject().value("text_color").toString();
    errColor  = localData->value("text_settings").toObject().value("err_color").toString();

    txtDocument->setMaximumBlockCount(localData->value("max_lines").toInt());
}

QString TextBody::htmlEsc(const QString &txt)
{
    QString ret = txt;

    ret.replace(">", "&gt;");
    ret.replace("<", "&lt;");

    return ret;
}

void TextBody::addTextBlock(const QString &txt, const QString &color)
{
    moveCursor(QTextCursor::End);

    QStringList lines = htmlEsc(txt).split("\n");

    for (int i = 0; i < lines.size(); ++i)
    {
        txtCursor->insertHtml("<div style=\"white-space: pre; color:" + color + ";\">" + lines[i] + "</div>");

        if (i != lines.size() - 1)
        {
            txtCursor->insertBlock();
        }
    }

    ensureCursorVisible();
}

void TextBody::setMaxLines(int value)
{
    txtDocument->setMaximumBlockCount(value);
}

void TextBody::addMainTxt(const QString &txt)
{
    addTextBlock(txt, mainColor);
}

void TextBody::addErrTxt(const QString &txt)
{
    addTextBlock(txt, errColor);
}

void TextBody::addBigTxt(const QString &txt)
{
    QString     lines;
    QTextStream txtOut(&lines);

    wordWrap("", txtOut, txt, Shared::mainWidget);

    addMainTxt(lines);
}

void TextBody::reload()
{
    clear();

    loadTextSettings(localData, this);
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
