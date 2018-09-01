/*
  Gpredict: Interface module for Gpredict by Alexandru Csete, OZ9AEC
  Gcontrols: Add-on module by Lisa Nelson AK7WS to control Radio
             and custom ASCII rotor.

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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QtSerialPort>
#include <QTimer>
#include <QtNetwork>
#include<QPlainTextEdit>
#include "datastructure.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString GetComData(QSerialPort *whatcom, QPlainTextEdit *where);
    void QStringReveal(QString &str);
    QString version = "1.16";

private slots:
    void on_pushButton_scan_com_ports_clicked();
    void on_pushButton_open_radio_port_clicked();
    void on_lineEdit_radio_port_returnPressed();
    void on_lineEdit_rotor_port_returnPressed();
    void on_pushButton_open_rotor_port_clicked();
    void slotTimerAlarm();

    void RadioNewNetConnection();
    void RadioSocketReadyRead();
    void RotorNewNetConnection();
    void RotorSocketReadyRead();

    void on_pushButton_read_radio_config_clicked();
    void on_pushButton_read_rotor_config_clicked();
    void SendDataToCom(DataStructure * , QSerialPort *, QString *, QPlainTextEdit *);
    void InserVariablesInString(QString *s);
    long RoundTo5k(long num);
    QString FreqBand(QString freq);

public slots:

private:
    Ui::MainWindow *ui;

    QTimer *timer;

    QSerialPort *RadioSerialPort;
    QSerialPort *RotorSerialPort;

    QTcpServer *RadioServer;
    QTcpServer *RotorServer;
    QTcpSocket *RadioSocket;
    QTcpSocket *RotorSocket;

    QStringList *fileList;

    DataStructure *RadioCode;
    DataStructure *RotorCode;

    bool DoRounding;
    long UplinkFreq;
    long DownlinkFreq;
    QString UplinkFreqS;
    QString DownlinkFreqS;


    QString LastUplinkFreq;
    QString LastDownlinkFreq;
    float Azumith;
    float Elevation;

public:

};

#endif // MAINWINDOW_H
