#include "command.h"

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

Command::Command(QObject *parent) : QObject(parent)
{
    // the QObject::objectName property determines the command
    // name for any object inheriting this object. avoid using
    // spaces and keep in mind that all command names are case
    // insensitive. avoid starting the command with the char
    // defined in CMD_ESCAPE, this char is used to send
    // commands directly to the host in case of naming
    // conflicts with the host commands.

    setObjectName("do_nothing");

    connect(this, &Command::setUserIO, this, &Command::setHook);
    connect(this, &Command::unsetUserIO, this, &Command::unsetHook);
}

void Command::setHook(int flgs)
{
    if (flgs & LOCAL_HOOK)
    {
        *Shared::clientHookedCmd = objectName();
    }
}

bool Command::activeHook()
{
    return *Shared::clientHookedCmd == objectName();
}

void Command::unsetHook(int flgs)
{
    if ((flgs & LOCAL_HOOK) && activeHook())
    {
        Shared::clientHookedCmd->clear();
    }
}

QString Command::shortText()
{
    // return a short summary on what this command actually does.
    // the client will use this to display help when '?' is
    // passed.

    return "";
}

QString Command::ioText()
{
    // just like shortText(), this is a help text command that
    // describes what input/output data to expect to/from this
    // command. the client will use the "[i]/[o]" format so it's
    // recommanded to display the text like this: [text]/[binary]
    // or you can be more descriptive like this:
    // [directory(text)]/[pcm_data(binary)].

    return "[none]/[none]";
}

QString Command::longText()
{
    // this is a help text function that is used by the client to
    // display full detailed information about the command and
    // it's usage. it's recommanded to be as thorough as possible
    // to help users understand proper usage of your command.

    return "";
}

QString Command::libText()
{
    // return the library name and version this command object
    // belongs to. there's no need to change this function's
    // output if creating an internal command.

    return QString(APP_NAME) + " v" + QCoreApplication::applicationVersion();
}

void Command::cmdCall(const QString &cmdName, const QString &argsLine)
{
    if (cmdName == objectName())
    {
        dataIn(argsLine);
        postExec();
    }
}

void Command::hookedCmdCall(const QString &argsLine)
{
    if (activeHook())
    {
        cmdCall(objectName(), argsLine);
    }
}

void Command::postExec()
{
    emit mainTxtOut("\nFinished: " + objectName() + "\n\n");
}
