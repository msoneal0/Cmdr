#    This file is part of Cmdr.
#
#    Cmdr is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Cmdr is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Cmdr under the LICENSE.md file. If not, see
#    <http://www.gnu.org/licenses/>.

QT += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

CONFIG -= console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.

DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = cmdr

LIBS += -lcrypto -lssl

SOURCES += src/main.cpp \
           src/cmd_line.cpp \
           src/cmd_objs/host_doc.cpp \
           src/common.cpp \
           src/main_ui.cpp \
           src/session.cpp \
           src/gen_file.cpp \
           src/text_body.cpp \
           src/cmd_objs/command.cpp \
           src/cmd_objs/info.cpp \
           src/cmd_objs/exec.cpp \
           src/cmd_objs/bookmarks.cpp \
           src/cmd_objs/status.cpp \
           src/cmd_objs/style.cpp

HEADERS += src/session.h \
           src/cmd_line.h \
           src/cmd_objs/host_doc.h \
           src/common.h \
           src/gen_file.h \
           src/main_ui.h \
           src/text_body.h \
           src/cmd_objs/command.h \
           src/cmd_objs/info.h \
           src/cmd_objs/exec.h \
           src/cmd_objs/bookmarks.h \
           src/cmd_objs/long_txt.h \
           src/cmd_objs/status.h \
           src/cmd_objs/style.h
