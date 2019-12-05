#include "main_ui.h"

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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *mainWidget = new QWidget(this);

    mainLayout = new QVBoxLayout(mainWidget);
    localData  = Shared::localData;

    QSize winSize;

    if (localData->contains("window_size"))
    {
        QJsonValue value = localData->value("window_size");

        if (value.isArray())
        {
            QJsonArray array = value.toArray();

            if (array.count() == 2)
            {
                winSize.setHeight(array[0].toInt());
                winSize.setWidth(array[1].toInt());
            }
        }
    }
    else
    {
        QScreen *screen = QGuiApplication::screens()[0];

        winSize.setHeight(screen->availableSize().height() / 2);
        winSize.setWidth(screen->availableSize().width() / 2);
    }

    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    resize(winSize);
    setCentralWidget(mainWidget);
    setContentsMargins(0, 0, 0, 0);
}

void MainWindow::saveWindowSize()
{
    QJsonArray array;

    array.append(height());
    array.append(width());

    localData->insert("window_size", array);

    saveLocalData(localData);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    saveWindowSize();

    QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    emit closeApp();

    event->ignore();
}

void MainWindow::setCmdLine(QWidget *widget)
{
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->insertWidget(1, widget);
}

void MainWindow::setTextBody(QWidget *widget)
{
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->insertWidget(0, widget);
}

void MainWindow::showUi()
{
    show();

    emit startup();
}
