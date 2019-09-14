#ifndef TEXT_BODY_H
#define TEXT_BODY_H

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

class TextBody : public QTextEdit
{
    Q_OBJECT

private:

    enum TextType
    {
        ERROR,
        MAIN
    };

    QList<QPair<TextType,QString> > textCache;
    QTextDocument                  *txtDocument;
    QTextCursor                    *txtCursor;
    QJsonObject                    *localData;
    QString                         errColor;
    QString                         mainColor;
    bool                            reloading;
    int                             maxLines;
    int                             lineCount;

    QString htmlEsc(const QString &txt);
    void    trimCache();
    void    loadTextBodySettings();
    void    contextMenuEvent(QContextMenuEvent *event);
    void    addTextBlock(const QString &txt, const QString &color, TextType tType);

private slots:

    void clearCache();

public:

    explicit TextBody(QWidget *parent = nullptr);

public slots:

    void addMainTxt(const QString &txt);
    void addErrTxt(const QString &txt);
    void addBigTxt(const QString &txt);
    void setMaxLines(int value);
    void reload();
};

#endif // TEXT_BODY_H
