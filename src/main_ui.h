#ifndef USER_IO_H
#define USER_IO_H

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:

    QJsonObject *localData;
    QVBoxLayout *mainLayout;

    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);
    void saveWindowSize();

public:

    explicit MainWindow(QWidget *parent = nullptr);

    void setCmdLine(QWidget *widget);
    void setTextBody(QWidget *widget);
    void showUi();

signals:

    void startup();
    void closeApp();
};

#endif // USER_IO_H
