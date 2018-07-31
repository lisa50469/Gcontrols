/*
  Gcontrol: Interface module for Gpredict by Alexandru Csete, OZ9AEC

  Copyright (C)  2018-2025  Lisa Nelson AK7WS.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, visit http://www.fsf.org/
*/
#include "mainwindow.h"
#include <QApplication>
#include <QSerialPort>
#include "datastructure.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

     MainWindow w;
    w.show();

    return a.exec();
}
