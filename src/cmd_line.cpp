#include "cmd_line.h"

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

CmdLine::CmdLine(QWidget *parent) : QComboBox(parent)
{
    localData = Shared::localData;
    flags     = 0;

    setEditable(true);
    setDuplicatesEnabled(true);
    loadTextSettings(localData, this);

    if (!localData->contains("command_hist"))
    {
        localData->insert("command_hist", QJsonArray());
    }

    if (!localData->contains("hist_limit"))
    {
        localData->insert("hist_limit", DEFAULT_HIST_LIMIT);
    }

    saveLocalData(localData);
    syncHistFromFile();

    connect(lineEdit(), SIGNAL(selectionChanged()), this, SLOT(selectionCheck()));
    connect(lineEdit(), SIGNAL(textEdited(QString)), this, SLOT(hideText(QString)));
    connect(lineEdit(), SIGNAL(textEdited(QString)), this, SLOT(checkForHistReset(QString)));
}

void CmdLine::setFlags(int flgs)
{
    flags |= flgs;
}

void CmdLine::unsetFlags(int flgs)
{
    flags ^= flgs;
}

void CmdLine::selectionCheck()
{
    if (flags & HIDDEN) lineEdit()->deselect();
}

void CmdLine::hideText(const QString &txt)
{
    if (flags & HIDDEN)
    {
        int currentPos = lineEdit()->cursorPosition();

        if (txt.size() > hiddenText.size())
        {
            int len = txt.size() - hiddenText.size();

            hiddenText.insert(currentPos - len, txt.mid(currentPos - len, len));

            lineEdit()->setText(QString(hiddenText.size(), '#'));
        }
        else if (hiddenText.size() > txt.size())
        {
            int len = hiddenText.size() - txt.size();

            hiddenText.remove(currentPos, len);
        }

        lineEdit()->setCursorPosition(currentPos);
    }
}

void CmdLine::checkForHistReset(const QString &txt)
{
    if (txt.isEmpty()) setCurrentIndex(-1);
}

QString CmdLine::cmdNameOnly(const QString &line)
{
    QString ret = line;
    int     pos = line.indexOf(" ");

    if (pos != -1)
    {
        ret = line.left(pos);
    }

    return ret.toLower();
}

void CmdLine::echo(const QString &line)
{
    if (!line.trimmed().isEmpty())
    {
        emit mainTxtOut(line + "\n\n");
    }
}

void CmdLine::clearHist()
{
    localData->insert("command_hist", QJsonArray());

    saveLocalData(localData);
    clear();

    lineEdit()->clear();
}

void CmdLine::syncHistToFile()
{
    QJsonArray hist;

    for (int i = 0; i < count(); ++i)
    {
        hist.append(itemData(i, Qt::DisplayRole).toString());
    }

    localData->insert("command_hist", hist);

    saveLocalData(localData);
}

void CmdLine::syncHistFromFile()
{
    clear();

    QJsonArray hist = localData->value("command_hist").toArray();

    for (auto item : hist)
    {
        addItem(item.toString());
    }

    setCurrentIndex(-1);
    setInsertPolicy(QComboBox::InsertAtTop);

    lineEdit()->clear();
}

void CmdLine::toHost(const QString &cmdName, const QString &args)
{
    if (flags & HOST_HOOK)
    {
        emit dataToHookedHost(toTEXT(args), TEXT);
    }
    else if (!cmdName.isEmpty())
    {
        quint16 cmdId = Shared::hostCmds->key(cmdName);

        if (!Shared::hostCmds->values().contains(cmdName))
        {
            emit errTxtOut("err: No such command: " + cmdName + "\n\n");
        }
        else if (Shared::genfileCmds->contains(cmdId))
        {
            emit dataToGenFile(cmdId, toTEXT(args));
        }
        else
        {
            emit dataToHost(cmdId, toTEXT(args), TEXT);
        }
    }
}

void CmdLine::toLocalCmd(const QString &cmdName, const QString &args)
{
    if (flags & LOCAL_HOOK)
    {
        emit dataToHookedCmdObj(args);
    }
    else
    {
        emit dataToCommandObj(cmdName, args);
    }
}

void CmdLine::procCmdLine(const QString &line)
{
    QString cmdName;
    QString argsLine;

    if (!(flags & (GEN_HOOK | HOST_HOOK | LOCAL_HOOK)))
    {
        cmdName  = cmdNameOnly(line.trimmed());
        argsLine = line.trimmed().mid(cmdName.size() + 1);
    }
    else if (Shared::hookBypass->contains(cmdNameOnly(line), Qt::CaseInsensitive))
    {
        cmdName  = cmdNameOnly(line.trimmed());
        argsLine = line.trimmed().mid(cmdName.size() + 1);
    }
    else if (flags & HIDDEN)
    {
        argsLine = hiddenText;
    }
    else
    {
        argsLine = line.trimmed();
    }

    if (flags & GEN_HOOK)
    {
        emit dataToHookedGenFile(toTEXT(argsLine));
    }
    else if (cmdName.startsWith(CMD_ESCAPE))
    {
        toHost(cmdName.mid(1), argsLine);
    }
    else if (Shared::clientCmds->contains(cmdName))
    {
        toLocalCmd(cmdName, argsLine);
    }
    else
    {
        toHost(cmdName, argsLine);
    }
}

void CmdLine::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = lineEdit()->createStandardContextMenu();

    menu->addAction("Clear History", this, SLOT(clearHist()));
    menu->exec(event->globalPos());
    menu->deleteLater();
}

void CmdLine::duplicateScan(const QString &txt)
{
    // this will remove any duplicates in the command line history, skipping over 1st line.
    // why not setDuplicatesEnabled(false)? the problem with that is it does an "inplace
    // replace" which means the duplicate item does not move from its position on the list
    // when inserted. i want this app to "float the duplicate to the top" when inserted
    // hence why i created this function.

    if (!(flags & HIDDEN))
    {
        for (int i = 1; i < count(); ++i)
        {
            if (itemText(i) == txt)
            {
                removeItem(i);
            }
        }
    }
}

void CmdLine::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return)
    {
        echo(lineEdit()->text());
        procCmdLine(lineEdit()->text());
        duplicateScan(lineEdit()->text());

        if (flags & HIDDEN)
        {
            removeItem(0);
            unsetFlags(HIDDEN);

            hiddenText.clear();
        }

        lineEdit()->clear();

        syncHistToFile();
        setCurrentIndex(-1);
    }
    else
    {
        QComboBox::keyReleaseEvent(event);
    }
}
