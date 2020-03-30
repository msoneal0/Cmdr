#include "bookmarks.h"

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

SaveBookmark::SaveBookmark(QObject *parent) : Command(parent)
{
    setObjectName("save_bookmark");

    if (!Shared::clientCmds->contains(objectName()))
    {
        Shared::clientCmds->insert(objectName(), this);
    }
}

QString SaveBookmark::shortText() {return tr("create or update a host bookmark.");}
QString SaveBookmark::ioText()    {return tr("[-name (text) {-addr (text)} {-port (text)} {-force}]/[text]");}
QString SaveBookmark::longText()  {return TXT_SaveBookmark;}

ListBookmarks::ListBookmarks(QObject *parent) : Command(parent)
{
    setObjectName("list_bookmarks");

    Shared::clientCmds->insert(objectName(), this);
}

QString ListBookmarks::shortText() {return tr("list all bookmarks available for loading.");}
QString ListBookmarks::ioText()    {return tr("[none]/[text]");}
QString ListBookmarks::longText()  {return TXT_ListBookmarks;}

DeleteBookmark::DeleteBookmark(QObject *parent) : Command(parent)
{
    setObjectName("delete_bookmark");

    Shared::clientCmds->insert(objectName(), this);
}

QString DeleteBookmark::shortText() {return tr("permanently delete a bookmark.");}
QString DeleteBookmark::ioText()    {return tr("[-name (text)]/[text]");}
QString DeleteBookmark::longText()  {return TXT_DeleteBookmark;}

SeeBookmark::SeeBookmark(QObject *parent) : Command(parent)
{
    setObjectName("view_bookmark");

    Shared::clientCmds->insert(objectName(), this);
}

QString SeeBookmark::shortText() {return tr("view the contents of a bookmark.");}
QString SeeBookmark::ioText()    {return tr("[-name (text)]/[text]");}
QString SeeBookmark::longText()  {return TXT_SeeBookmark;}

void SaveBookmark::term()
{
    baseArgs.clear();
    baseName.clear();

    emit unsetUserIO(LOCAL_HOOK);
}

void SaveBookmark::run(const QString &name, QStringList &args)
{
    QFile file(appDataDir() + BOOKMARK_FOLDER + "/" + name + ".json");

    QJsonObject json;

    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        auto addr = getParam("-addr", args);
        auto port = getParam("-port", args).toInt();

        if (addr.isEmpty()) addr = *Shared::hostAddress;
        if (port == 0)      port = *Shared::hostPort;

        json.insert("address", addr);
        json.insert("port", port);

        QJsonDocument doc(json);

        file.write(doc.toJson());
    }
    else
    {
        cacheTxt(ERR, "err: Could not open the bookmark file for writing, reason: " + file.errorString() + "\n");
    }

    file.close();
}

void SaveBookmark::askOverwrite()
{
    cacheTxt(TEXT, "'" + baseName + "' already exists. do you want to overwrite? (y/n): ");

    emit setUserIO(LOCAL_HOOK);
}

void SaveBookmark::dataIn(const QString &argsLine)
{
    if (activeHook())
    {
        if (QRegExp("y", Qt::CaseInsensitive).exactMatch(argsLine))
        {
            run(baseName, baseArgs);
            term();
        }
        else if (QRegExp("n", Qt::CaseInsensitive).exactMatch(argsLine))
        {
            term();
        }
        else
        {
            askOverwrite();
        }
    }
    else
    {
        auto args  = parseArgs(argsLine);
        auto force = argExists("-force", args);
        auto name  = getParam("-name", args);

        if (name.isEmpty())
        {
            cacheTxt(ERR, "err: Bookmark name (-name) not given.\n");
        }
        else
        {
            baseName = name;
            baseArgs = args;

            if (QFile(appDataDir() + BOOKMARK_FOLDER + "/" + name + ".json").exists())
            {
                if (force) run(name, args);
                else       askOverwrite();
            }
            else
            {
                run(name, args);
            }
        }
    }
}

void ListBookmarks::dataIn(const QString &argsLine)
{
    Q_UNUSED(argsLine)

    QStringList list = QDir(appDataDir() + BOOKMARK_FOLDER).entryList(QStringList() << "*.json", QDir::Files, QDir::Name);

    if (list.isEmpty())
    {
        cacheTxt(TEXT, "Empty...\n");
    }
    else
    {
        cacheTxt(TEXT, "Bookmarks:\n\n");

        for (int i = 0; i < list.size(); ++i)
        {
            cacheTxt(TEXT, " " + QFileInfo(list[i]).baseName() + "\n");
        }
    }
}

void DeleteBookmark::dataIn(const QString &argsLine)
{
    auto args = parseArgs(argsLine);
    auto name = getParam("-name", args);

    if (name.isEmpty())
    {
        cacheTxt(ERR, "err: Bookmark name (-name) not given.\n");
    }
    else
    {
        auto path = appDataDir() + BOOKMARK_FOLDER + "/" + name + ".json";

        if (!QFile::remove(path))
        {
            cacheTxt(ERR, "err: Could not delete the bookmark, the file might not exist or you don't have write permissions.\n");
        }
    }
}

void SeeBookmark::dataIn(const QString &argsLine)
{
    auto args = parseArgs(argsLine);
    auto name = getParam("-name", args);

    if (name.isEmpty())
    {
        cacheTxt(ERR, "err: Bookmark name (-name) not given.\n");
    }
    else
    {
        auto path = appDataDir() + BOOKMARK_FOLDER + "/" + name + ".json";

        QFile file(path, this);

        if (file.open(QFile::ReadOnly))
        {
            auto doc = QJsonDocument::fromJson(file.readAll());

            cacheTxt(TEXT, "address: " + doc.object().value("address").toString() + "\n");
            cacheTxt(TEXT, "port:    " + QString::number(doc.object().value("port").toInt()) + "\n");
        }
        else
        {
            cacheTxt(ERR, "err: Could not open the requested bookmark file for reading, reason: " + file.errorString() + '\n');
        }

        file.close();
    }
}
