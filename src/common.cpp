#include "common.h"

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

void setupTextSettings(QJsonObject *data)
{
    QString txtColorStr = "#FBFBD7";
    QString bgColorStr  = "#363A49";
    QString errColorStr = "#BB55D8";
    QString fontFamily  = "courier";
    QString fontSize    = "9";

    QJsonObject obj;

    if (data->contains("text_settings"))
    {
        obj = data->value("text_settings").toObject();

        if (!obj.contains("text_color"))  obj.insert("text_color", txtColorStr);
        if (!obj.contains("err_color"))   obj.insert("err_color", errColorStr);
        if (!obj.contains("font_family")) obj.insert("font_family", fontFamily);
        if (!obj.contains("bg_color"))    obj.insert("bg_color", bgColorStr);
        if (!obj.contains("font_size"))   obj.insert("font_size", fontSize);
    }
    else
    {
        obj.insert("text_color", txtColorStr);
        obj.insert("err_color", errColorStr);
        obj.insert("font_family", fontFamily);
        obj.insert("bg_color", bgColorStr);
        obj.insert("font_size", fontSize);
    }

    data->insert("text_settings", obj);

    if (!data->contains("max_lines"))
    {
        data->insert("max_lines", DEFAULT_MAX_LINES);
    }

    saveLocalData(data);
}

void loadTextSettings(QJsonObject *data, QWidget *widget)
{
    QString  bgColor   = data->value("text_settings").toObject().value("bg_color").toString();
    QString  txtColor  = data->value("text_settings").toObject().value("text_color").toString();
    QString  fntFamily = data->value("text_settings").toObject().value("font_family").toString();
    QString  fntSize   = data->value("text_settings").toObject().value("font_size").toString();
    QPalette pal       = widget->palette();
    QFont    fnt       = widget->font();

    pal.setColor(QPalette::Active, QPalette::Base, QColor(bgColor));
    pal.setColor(QPalette::Inactive, QPalette::Base, QColor(bgColor));
    pal.setColor(QPalette::Active, QPalette::Text, QColor(txtColor));
    fnt.setFamily(fntFamily);
    fnt.setPointSize(fntSize.toInt());

    widget->setPalette(pal);
    widget->setFont(fnt);
}

QString appDataDir()
{
    QString path = QDir::homePath() + "/";

#ifndef Q_OS_WIN32

    path.append("." + QString(APP_TARGET) + "/");

#else

    path.append(QString("AppData/Local/") + QString(APP_TARGET) + "/");

#endif

    QDir dir;

    dir.mkpath(path);
    dir.mkpath(path + BOOKMARK_FOLDER);

    return path;
}

void saveLocalData(QJsonObject *obj)
{
    QFile file(appDataDir() + CONFIG_FILENAME);

    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        file.write(QJsonDocument(*obj).toJson());
    }

    file.close();
}

void loadLocalData(QJsonObject *obj)
{
    QFile file(appDataDir() + CONFIG_FILENAME);

    if (file.open(QFile::ReadOnly))
    {
        *obj = QJsonDocument::fromJson(file.readAll()).object();
    }

    file.close();
}

QByteArray wrFrame(quint16 cmdId, const QByteArray &data, uchar dType)
{
    QByteArray typeBa = wrInt(dType, 8);
    QByteArray cmdBa  = wrInt(cmdId, 16);
    QByteArray branBa = wrInt(0, 16);
    QByteArray sizeBa = wrInt(static_cast<quint64>(data.size()), 24);

    return typeBa + cmdBa + branBa + sizeBa + data;
}

QByteArray toTEXT(const QString &txt)
{
    QByteArray ret = QTextCodec::codecForName(TXT_CODEC)->fromUnicode(txt);

    return ret.mid(2);
}

QByteArray fixedToTEXT(const QString &txt, int len)
{
    return toTEXT(txt).leftJustified(len, 0, true);
}

QString fromTEXT(const QByteArray &txt)
{
    QByteArray ba = txt;

    ba.replace(QByteArray(2, 0x00), QByteArray());

    return QTextCodec::codecForName(TXT_CODEC)->toUnicode(ba);
}

QByteArray wrInt(quint64 num, int numOfBits)
{
    // convert quint64 values into little endian QByteArrays.
    // if any invalid numOfBits is passed the function will
    // return an empty QByteArray. vaild range: 1-64.

    // also note that this function will truncate in a per byte
    // bases but not in a per bit bases.

    // eg. wrInt(1024, 8) will return 0.
    //                    1024 (0x00, 0x04) to 8 bits (0x00)
    // eg. wrInt(255, 2)  will still return 255.
    //                    255 (0xFF) to 2 bits (0xFF)

    QByteArray ret;
    quint64    leNum = qToLittleEndian(num);
    char      *ptr   = static_cast<char*>(static_cast<void*>(&leNum));

    if (numOfBits < 8)   numOfBits = 8;
    if (numOfBits <= 64) ret.append(ptr, numOfBits / 8);

    return ret;
}

quint64 rdInt(const QByteArray &bytes)
{
    // convert little endian QByteArrays into local sytem
    // endianness uints.

    QByteArray ba  = bytes.leftJustified(sizeof(quint64), 0);
    void      *vod = reinterpret_cast<void*>(ba.data());

    return qFromLittleEndian<quint64>(vod);
}

QString extractCmdName(const QByteArray &data)
{
    QStringList args = parseArgs(data, 1);
    QString     ret;

    if (!args.isEmpty())
    {
        ret = args[0];
    }

    return ret;
}

QStringList parseArgs(const QByteArray &data, int maxArgs)
{
    QStringList ret;
    QString     arg;
    QString     line      = fromTEXT(data);
    bool        inDQuotes = false;
    bool        inSQuotes = false;
    bool        escaped   = false;

    for (int i = 0; i < line.size(); ++i)
    {
        if ((line[i] == '\'') && !inDQuotes && !escaped)
        {
            // single quote '

            inSQuotes = !inSQuotes;
        }
        else if ((line[i] == '\"') && !inSQuotes && !escaped)
        {
            // double quote "

            inDQuotes = !inDQuotes;
        }
        else
        {
            escaped = false;

            if (line[i].isSpace() && !inDQuotes && !inSQuotes)
            {
                // space

                if (!arg.isEmpty())
                {
                    ret.append(arg);
                    arg.clear();
                }
            }
            else
            {
                if ((line[i] == '\\') && ((i + 1) < line.size()))
                {
                    if ((line[i + 1] == '\'') || (line[i + 1] == '\"'))
                    {
                        escaped = true;
                    }
                    else
                    {
                        arg.append(line[i]);
                    }
                }
                else
                {
                    arg.append(line[i]);
                }
            }
        }

        if ((ret.size() >= maxArgs) && (maxArgs != -1))
        {
            break;
        }
    }

    if (!arg.isEmpty() && !inDQuotes && !inSQuotes)
    {
        ret.append(arg);
    }

    return ret;
}

QStringList parseArgs(const QString &line)
{
    return parseArgs(toTEXT(line), -1);
}

void wordWrap(const QString &label, QTextStream &txtOut, const QString &txtIn, QWidget *measureWid)
{
    // this breaks up a long text line into smaller lines of text
    // equal to or less then the given len. this also avoids line
    // breaking mid-word and also honors line breaks by treating
    // lines with line breaks as complete text bodies.

    int         width      = measureWid->width() / 8;
    bool        labelWr    = true;
    QString     indent     = QString(label.size(), ' ');
    QStringList textBodies = txtIn.split('\n');

    for (int i = 0; i < textBodies.size(); ++i)
    {
        int         index = 0;
        QStringList words = textBodies[i].split(' ');
        QString     line;

        while(index < words.size())
        {
            line.append(words[index] + " ");

            if ((index == (words.size() - 1)) && !line.isEmpty())
            {
                // add the last remaining line if there are no more
                // words left to add.

                line.chop(1);

                if (labelWr) txtOut << label << line << endl;
                else         txtOut << indent << line << endl;

                labelWr = false;
            }
            else if ((line.size() + words[index + 1].size() + 1) >= width)
            {
                // check ahead in the words list to see if the next
                // word would cause the line to be >= len. if that is
                // the case, the line is added to the return and then
                // cleared.

                if (labelWr) txtOut << label << line << endl;
                else         txtOut << indent << line << endl;

                line.clear();

                labelWr = false;
            }

            index++;
        }
    }
}

QString boolText(bool state)
{
    if (state) return QString("True");
    else       return QString("False");
}

QString verText(quint16 maj, quint16 min, quint16 patch)
{
    return QString::number(maj) + "." + QString::number(min) + "." + QString::number(patch);
}

QString getParam(const QString &key, const QStringList &args)
{
    // this can be used by command objects to pick out parameters
    // from a command line that are pointed by a name identifier
    // example: -i /etc/some_file, this function should pick out
    // "/etc/some_file" from args if "-i" is passed into key.

    QString ret;

    int pos = args.indexOf(QRegExp(key, Qt::CaseInsensitive));

    if (pos != -1)
    {
        // key found.

        if ((pos + 1) <= (args.size() - 1))
        {
            // check ahead to make sure pos + 1 will not go out
            // of range.

            if (!args[pos + 1].startsWith("-"))
            {
                // the "-" used throughout this application
                // indicates an argument so the above 'if'
                // statement will check to make sure it does
                // not return another argument as a parameter
                // in case a back-to-back "-arg -arg" is
                // present.

                ret = args[pos + 1];
            }
        }
    }

    return ret;
}

bool argExists(const QString &key, const QStringList &args)
{
    return args.contains(key, Qt::CaseInsensitive);
}

bool                     *Shared::connectedToHost = nullptr;
bool                     *Shared::activeDisp      = nullptr;
QJsonObject              *Shared::localData       = nullptr;
QHash<QString, Command*> *Shared::clientCmds      = nullptr;
QHash<quint16, QString>  *Shared::hostCmds        = nullptr;
QHash<quint16, QString>  *Shared::genfileCmds     = nullptr;
QHash<QString, Command*> *Shared::hostDocs        = nullptr;
QStringList              *Shared::hookBypass      = nullptr;
QByteArray               *Shared::sessionId       = nullptr;
QString                  *Shared::clientHookedCmd = nullptr;
ushort                   *Shared::servMajor       = nullptr;
ushort                   *Shared::servMinor       = nullptr;
ushort                   *Shared::servPatch       = nullptr;
QString                  *Shared::hostAddress     = nullptr;
quint16                  *Shared::hostPort        = nullptr;
quint16                  *Shared::termCmdId       = nullptr;
QSslSocket               *Shared::socket          = nullptr;
CmdLine                  *Shared::cmdLine         = nullptr;
Genfile                  *Shared::genFile         = nullptr;
MainWindow               *Shared::mainWin         = nullptr;
Session                  *Shared::session         = nullptr;
TextBody                 *Shared::textBody        = nullptr;
TextWorker               *Shared::textWorker      = nullptr;
ContextReloader          *Shared::contextReloader = nullptr;
QWidget                  *Shared::mainWidget      = nullptr;
QList<quint8>            *Shared::idCache         = nullptr;
QList<QString>           *Shared::txtCache        = nullptr;
ThreadKiller             *Shared::theadKiller     = nullptr;

bool Shared::cacheTxt(CacheOp op, quint8 &typeId, QString &txt)
{
    QMutex mutex;
    bool   ret = false;

    mutex.lock();

    if (op == TXT_IN)
    {
        if ((typeId == TEXT) || (typeId == ERR) || (typeId == BIG_TEXT) || (typeId == PRIV_TEXT))
        {
            idCache->append(typeId);
            txtCache->append(txt);

            ret = true;
        }
    }
    else if (op == TXT_OUT)
    {
        if (!idCache->isEmpty() && !txtCache->isEmpty())
        {
            typeId = idCache->takeFirst();
            txt    = txtCache->takeFirst();
            ret    = true;
        }
    }
    else if (op == TXT_CLEAR)
    {
        *Shared::activeDisp = false;

        idCache->clear();
        txtCache->clear();
    }
    else if (op == TXT_IS_EMPTY)
    {
        ret = idCache->isEmpty() && txtCache->isEmpty();
    }

    mutex.unlock();

    return ret;
}

bool Shared::cacheTxt(CacheOp op)
{
    quint8  unusedId = 0;
    QString unusedTxt;

    return cacheTxt(op, unusedId, unusedTxt);
}

ThreadKiller::ThreadKiller(QObject *parent) : QObject(parent)
{
    threadCount = 3;
}

void ThreadKiller::threadFinished()
{
    threadCount--;

    if (threadCount == 0)
    {
        QCoreApplication::instance()->quit();
    }
}
