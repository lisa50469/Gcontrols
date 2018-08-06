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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datastructure.h"
#include <QTcpSocket>
#include <QDir>

//-----------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Gcontrol by Lisa Nelson Version: "+ version);

    QDirIterator Radioit(".", QStringList() << "*.rad", QDir::Files, QDirIterator::Subdirectories);
    while (Radioit.hasNext()) {
        qDebug() << Radioit.next();
        ui->comboBox_Radio_File_Config->addItem(Radioit.fileName());
    }

    QDirIterator Rotorit(".", QStringList() << "*.rot", QDir::Files, QDirIterator::Subdirectories);
    while (Rotorit.hasNext()) {
        qDebug() << Rotorit.next();
        ui->comboBox_Rotor_File_Config->addItem(Rotorit.fileName());
    }

    RadioSerialPort = new QSerialPort(this);
    RotorSerialPort = new QSerialPort(this);
    RadioServer = new QTcpServer;
    RotorServer = new QTcpServer;

    Azumith = 21;
    Elevation = 22;
    UplinkFreq = 123000000;
    DownlinkFreq = 456000000;
    MainWindow::DoRounding = false;

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
    timer->start(500);

    RadioCode = new DataStructure;
    RotorCode = new DataStructure;

}

//-----------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

//-----------------------------------------------------------------------
void MainWindow::on_pushButton_scan_com_ports_clicked()
{
    ui->comboBox_radio_port->clear();
    ui->comboBox_rotor_port->clear();
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts())
        {
        ui->comboBox_radio_port->addItem(port.portName());
        ui->comboBox_rotor_port->addItem(port.portName());
        }
}

//-----------------------------------------------------------------------
void MainWindow::on_pushButton_open_radio_port_clicked()
{
    if (RadioSerialPort->isOpen())
        {
        qDebug() << "Closing Radio Com port...";
        RadioSerialPort->close();
        ui->pushButton_open_radio_port->setText("Port Closed");
        ui->lineEdit_radio_port->setEnabled(0);

        if ((RadioSocket->ConnectedState == QAbstractSocket::UnconnectedState) &&
                (RadioSocket->isOpen())) // Close the network operations when com is closed
            {
            RadioSocket->close();
            }

        if (RadioServer->isListening())
            {
            RadioServer->close();
            ui->plainTextEditRadio->setEnabled(0);
            }
        else
            {
            ui->plainTextEditRadio->setEnabled(0);
            }

        }
    else
        {
        RadioSerialPort = new QSerialPort(this);
        RadioSerialPort->setPortName(ui->comboBox_radio_port->currentText());
        if (RadioSerialPort->open(QIODevice::ReadWrite))
            {
            RadioSerialPort->setBaudRate(QSerialPort::Baud9600);
            RadioSerialPort->setDataBits(QSerialPort::Data8);
            RadioSerialPort->setFlowControl(QSerialPort::NoFlowControl);
            RadioSerialPort->setStopBits(QSerialPort::OneStop);
            ui->pushButton_open_radio_port->setText("Port Open");
            ui->lineEdit_radio_port->setEnabled(1);
            ui->plainTextEditRadio->setEnabled(1);

            RadioInitialization();  // Send the data from the config file that was read.

            // Now enable the NET server for the Radio
            RadioServer->listen(QHostAddress::Any,ui->lineEditRadioNetPort->text().toInt());
            connect(RadioServer,SIGNAL(newConnection()),this,SLOT(RadioNewNetConnection()));
            }
        else
            {
            qDebug() << "Radio Com Port FAILED!";
            ui->lineEdit_radio_port->setEnabled(0);
            ui->plainTextEditRadio->setEnabled(0);
            }
        }
}

//-----------------------------------------------------------------------
void MainWindow::on_pushButton_open_rotor_port_clicked()
{
    if (RotorSerialPort->isOpen())  // Port is already open
        {
        RotorSerialPort->close();
        ui->pushButton_open_rotor_port->setText("Port Closed");
        ui->lineEdit_rotor_port->setEnabled(0);

        if ((RotorSocket->ConnectedState == QAbstractSocket::UnconnectedState) &&
                (RotorSocket->isOpen())) // Close the network operations when com is closed
            {
            RotorSocket->close();
            }

        if (RotorServer->isListening())
            {
            RotorServer->close();
            ui->plainTextEditRotor->setEnabled(0);
            }
        else
            {
            ui->plainTextEditRotor->setEnabled(0);
            }

        }
    else    // Com port is not open.
        {
        RotorSerialPort->setPortName(ui->comboBox_rotor_port->currentText());
        qDebug() << RotorSerialPort->portName();
        if (RotorSerialPort->open(QIODevice::ReadWrite))
            {
            RotorSerialPort->setBaudRate(QSerialPort::Baud9600);
            RotorSerialPort->setDataBits(QSerialPort::Data8);
            RotorSerialPort->setFlowControl(QSerialPort::NoFlowControl);
            RotorSerialPort->setStopBits(QSerialPort::OneStop);
            ui->pushButton_open_rotor_port->setText("Port Open");
            ui->lineEdit_rotor_port->setEnabled(1);
            ui->plainTextEditRotor->setEnabled(1);

            RotorInitialization();  // Send the data from the config file that was read.

            // Now enable the NET server for the Rotor
            //RotorServer = new QTcpServer;
            RotorServer->listen(QHostAddress::Any,ui->lineEditRotorNetPort->text().toInt());
            connect(RotorServer,SIGNAL(newConnection()),this,SLOT(RotorNewNetConnection()));
            }
        else
            {
            qDebug() << "Rotor Com Port FAILED!";
            ui->lineEdit_rotor_port->setEnabled(0);
            ui->plainTextEditRotor->setEnabled(0);
            }
        }
}

//-----------------------------------------------------------------------
void MainWindow::QStringReveal(QString &str)
{
    str.replace( '\r', "\\r" );
    str.replace( '\n', "\\n" );
}

//-----------------------------------------------------------------------
void MainWindow::on_lineEdit_radio_port_returnPressed()
{
    QString str = ui->lineEdit_radio_port->text();
    str += RadioCode->ComTermChars;
    //qDebug() << "simplified:" << str;
    RadioSerialPort->write(str.toUtf8());
    MainWindow::QStringReveal(str);
    if(ui->checkBox_logging->checkState())
        ui->plainTextEditRadio->appendPlainText(">: "+str);
}

//-----------------------------------------------------------------------
void MainWindow::on_lineEdit_rotor_port_returnPressed()
{
    QByteArray data = ui->lineEdit_rotor_port->text().toLocal8Bit();
    QString str = data;
    data += '\n';   // Need to define termination in the config file too
    RotorSerialPort->write(data);
    str += '\n';
    MainWindow::QStringReveal(str);
    if(ui->checkBox_logging->checkState())
        ui->plainTextEditRotor->appendPlainText(">: "+str);
    //qDebug() << "Wrote to rotor Com port:" << str;
}

//-----------------------------------------------------------------------
QString MainWindow::GetComData(QSerialPort *whatcom,QPlainTextEdit *where)
{
    QString str;
    if (whatcom->isOpen())
        str = whatcom->readAll();

    if (str.length()>0)
        {
        //qDebug() << "Com Port Incoming Data: " << str;
        MainWindow::QStringReveal(str);
        if(ui->checkBox_logging->checkState())
            where->appendPlainText("<: "+str);
        }
return str;
}

//-----------------------------------------------------------------------
void MainWindow::slotTimerAlarm()
{
    MainWindow::GetComData(RadioSerialPort,ui->plainTextEditRadio);
    MainWindow::GetComData(RotorSerialPort,ui->plainTextEditRotor);

    if (RotorSocket->ConnectedState == QAbstractSocket::UnconnectedState)
        qDebug() << "Rotor socket is not connected";
    //if (!RotorServer->isListening())
     //   qDebug() << "Rotor server is not connected";
    if (RadioSocket->ConnectedState == QAbstractSocket::UnconnectedState)
        qDebug() << "Radio socket is not connected";
   // if (!RadioServer->isListening())
    //    qDebug() << "Radio server is not connected";

    char str[20];
    sprintf(str,"%ld",UplinkFreq);
    ui->lineEdit_Uplink_Freq->setText(str);
    sprintf(str,"%ld",DownlinkFreq);
    ui->lineEdit_Downlink_Freq->setText(str);
}

//-----------------------------------------------------------------------
void MainWindow::RadioNewNetConnection()
{
    RadioSocket = RadioServer->nextPendingConnection();
    if (!RadioSocket)
    {
        qDebug() << "Socket failed to open.";
        return;
    }
    qDebug() << "New Radio Net connection.";
    //ui->plainTextEditRadioNet->setEnabled(1);
    connect(RadioSocket,SIGNAL(readyRead()),this,SLOT(RadioSocketReadyRead()));
}

//-----------------------------------------------------------------------
void MainWindow::RotorNewNetConnection()
{
    RotorSocket = RotorServer->nextPendingConnection();
    if (!RotorSocket)
    {
        qDebug() << "Rotor Socket failed to open.";
        return;
    }
    qDebug() << "New Rotor Net connection.";
    //ui->plainTextEditRotorNet->setEnabled(1);
    connect(RotorSocket,SIGNAL(readyRead()),this,SLOT(RotorSocketReadyRead()));
}

//-----------------------------------------------------------------------
long MainWindow::RoundTo5000(long num)
{
    int remainder = num % 10000;
    int bigpart = num / 10000;

    if (remainder < 5000)
    {
     if (remainder < 2500)
        return bigpart*10000;
     else
        return bigpart*10000+5000;
    }
    else
    {
    if(remainder < 7500)
        return bigpart*10000+5000;
    else
        return (bigpart+1)*10000;
    }
}

//-----------------------------------------------------------------------
void MainWindow::RadioSocketReadyRead()
{
    QString s,s2;
    char str[50];
    char str2[50];
    char c;
    QByteArray data = RadioSocket->readAll();
    //qDebug() << "Radio Data In:" << data;
    int i;
    for (i=0;i<data.size();i++)
        str[i] = data[i];
    str[i] = '\0'; // Terminate the string

    i = data[0];
    s = data;
    MainWindow::QStringReveal(s);
    switch (i) {
    case 'S':
        s = "RPRT 0\r";
        RadioSocket->write(s.toUtf8());
        MainWindow::QStringReveal(s);
        break;
    case 'I':
        sscanf(str,"%c %ld",&c,&UplinkFreq);
        //qDebug() << "New uplink freq: " << UplinkFreq;
        s = "RPRT 0\r";
        RadioSocket->write(s.toUtf8());
        MainWindow::QStringReveal(s);
        if(ui->checkBox_logging->checkState())
            ui->plainTextEditRadio->appendPlainText(">: "+s);

        RadioCode->GetResponseCode("FREQFORMAT",&s);
        for (i=0;i<s.toUtf8().size();i++)
            str2[i] = s.toUtf8()[i];
        str2[i] = '\0'; // Terminate the string
        sprintf(str,str2,MainWindow::RoundTo5000(UplinkFreq));
        if (MainWindow::DoRounding)
            sprintf(str,str2,MainWindow::RoundTo5000(UplinkFreq));
        else
            sprintf(str,str2,UplinkFreq);
        RadioCode->GetResponseCode("SETUP",&s);
        s2 = str;
        s.replace( "%freq%", s2);
        s += RadioCode->ComTermChars;
        RadioSerialPort->write(s.toUtf8());
        MainWindow::QStringReveal(s);
        if(ui->checkBox_logging->checkState())
            ui->plainTextEditRadio->appendPlainText(">: "+s);
        break;
    case 'F':
        sscanf(str,"%c %ld",&c,&DownlinkFreq);
        s = "RPRT 0\r";
        RadioSocket->write(s.toUtf8());
        MainWindow::QStringReveal(s);
        if(ui->checkBox_logging->checkState())
            ui->plainTextEditRadio->appendPlainText(">: "+s);

        RadioCode->GetResponseCode("FREQFORMAT",&s);
        for (i=0;i<s.toUtf8().size();i++)
            str2[i] = s.toUtf8()[i];
        str2[i] = '\0'; // Terminate the string
        if (MainWindow::DoRounding)
            sprintf(str,str2,MainWindow::RoundTo5000(DownlinkFreq));
        else
            sprintf(str,str2,DownlinkFreq);
        RadioCode->GetResponseCode("SETDN",&s);
        s2 = str;
        s.replace( "%freq%", s2);
        s += RadioCode->ComTermChars;
        RadioSerialPort->write(s.toUtf8());
        MainWindow::QStringReveal(s);
        if(ui->checkBox_logging->checkState())
            ui->plainTextEditRadio->appendPlainText(">: "+s);
        break;
    case 'f':
        sprintf(str,"%ld",DownlinkFreq);
        strcat(str,"\r");
        s = str;
        RadioSocket->write(s.toUtf8());
        break;
    case 'i':
        sprintf(str,"%ld",UplinkFreq);
        strcat(str,"\r");
        s = str;
        RadioSocket->write(s.toUtf8());
        MainWindow::QStringReveal(s);
        break;
    default:
        qDebug() << "Default Radio In:" << data;

        break;
    }
}

//-----------------------------------------------------------------------
void MainWindow::RotorSocketReadyRead()
{
    QString s;
    char str[50];
    char az[50];
    char el[50];
    char c;
    QByteArray data = RotorSocket->readAll();
    int i;
    i = data[0];
    s = data;
    MainWindow::QStringReveal(s);

    switch (i) {
    case 'p':
        sprintf(az,"%.2f",Azumith);
        sprintf(el,"%.2f",Elevation);
        s = "%AZ%\n%EL%\n";
        s.replace( "%AZ%", az);
        s.replace( "%EL%", el);
        RotorSocket->write(s.toUtf8());
        break;
    case 'P':
        sscanf(data,"%c %f %f",&c,&Azumith,&Elevation);
        RotorSocket->write("RPRT 0\n");
        sprintf(str,"AZ%.2f EL%.2f",Azumith,Elevation);
        s = str;
        s += RotorCode->ComTermChars;
qDebug() << "Sending this to com port:" << s.toUtf8();
        RotorSerialPort->write(s.toUtf8());
        MainWindow::QStringReveal(s);
        if(ui->checkBox_logging->checkState())
            ui->plainTextEditRotor->appendPlainText(">: "+s);
        break;
    case 'S':
        RotorSocket->write("RPRT 0\n");
        break;
    default:
        break;
    }
}

//-----------------------------------------------------------------------
void MainWindow::on_pushButton_read_radio_config_clicked()
{
    QString cmd,s1,s2,line;
    //QFile file("/home/lisa/src/Gcontrols/v71a.rad"); // XXX  Need to get the file name.
    QFile file(ui->comboBox_Radio_File_Config->currentText()); // XXX  Need to get the file name.
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         return;

     QTextStream in(&file);
     while  (!in.atEnd())
        {
            line = in.readLine();
            //qDebug() << "New Line:" << line;

            cmd = line.split(":").at(0);
            if (cmd == "TERM")
                {
                s1 = line.split(":").at(1);
                QChar c = s1.toInt();
                RadioCode->ComTermChars = c;
                }
            if (cmd == "ROUND")
                {
                //MainWindow::DoRounding = true;
                DoRounding = true;
                qDebug() << "Rounding is turned on.";
                }
            if (cmd == "DIGITS")
                {
                s1 = line.split(":").at(1);
                NumDigits = s1.toInt();
                qDebug() << "number of digits set to " << NumDigits;
                }
            if (cmd == "INIT")
            {
                s1 = line.split(":").at(1);
                RadioCode->AddStructInits(s1);
            }
            if (cmd == "RESP")
            {
                s1 = line.split(":").at(1);
                s2 = line.split(":").at(2);
                RadioCode->AddStructResps(s1,s2);
            }
        };
     RadioCode->PrintInits();
     RadioCode->PrintResps();
}

//-----------------------------------------------------------------------
void MainWindow::RadioInitialization()
{
    //qDebug() << "Initializing Radio com...";
    QString s;
    InitStruct *ptr = RadioCode->Init;  // Start at the beginning.
    //qDebug() << "------- Sending Init data ----------";
    //qDebug() << "Termination Characters: " << RadioCode->ComTermChars;
    QByteArray data;
    while (ptr != NULL)
    {
        data = ptr->str.toUtf8();
        data += RadioCode->ComTermChars;
        s = data;
        MainWindow::QStringReveal(s);
        if(ui->checkBox_logging->checkState())
            ui->plainTextEditRadio->appendPlainText(">: "+s);
        RadioSerialPort->write(data);
        //qDebug() << "Sending data to com port:" << data;
        ptr = ptr->next;
    }
}

//-----------------------------------------------------------------------
void MainWindow::RotorInitialization()
{
    qDebug() << "Initializing Roror com...";
    QString s;
    InitStruct *ptr = RotorCode->Init;  // Start at the beginning.
    qDebug() << "------- Sending Init data ----------";
    qDebug() << "Termination Characters: " << RotorCode->ComTermChars;
    QByteArray data;
    while (ptr != NULL)
    {
        data = ptr->str.toUtf8();
        data += RotorCode->ComTermChars;
        s = data;
        MainWindow::QStringReveal(s);
        if(ui->checkBox_logging->checkState())
            ui->plainTextEditRotor->appendPlainText(">: "+s);
        RotorSerialPort->write(data);
        //qDebug() << "Sending data to com port:" << data;
        ptr = ptr->next;
    }
}

//-----------------------------------------------------------------------
void MainWindow::on_pushButton_read_rotor_config_clicked()
{
    QString cmd,s1,s2,line;
    QFile file(ui->comboBox_Rotor_File_Config->currentText()); // XXX  Need to get the file name.
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         return;
qDebug() << "Reading rotor config file.";
     QTextStream in(&file);
     while  (!in.atEnd())
        {
            line = in.readLine();
            qDebug() << "New Line:" << line;

            cmd = line.split(":").at(0);
            if (cmd == "TERM")
                {
                s1 = line.split(":").at(1);
                qDebug() << s1;
                QChar c = s1.toInt();
                RotorCode->ComTermChars = c;
                }
            if (cmd == "INIT")
            {
                s1 = line.split(":").at(1);
                RotorCode->AddStructInits(s1);
            }
            if (cmd == "RESP")
            {
                s1 = line.split(":").at(1);
                s2 = line.split(":").at(2);
                RotorCode->AddStructResps(s1,s2);
            }
        };
     RotorCode->PrintInits();
     RotorCode->PrintResps();
}

//-----------------------------------------------------------------------
void MainWindow::InserVariablesInString(QString *s)
{

    if (s->contains("%uplink%")>0)
       s->replace( "%uplink%", QString::number(UplinkFreq) );
    if (s->contains("%downlink%")>0)
        s->replace( "%downlink%", QString::number(DownlinkFreq) );

    if (s->contains("%az%")>0)
        s->replace( "%az%", QString::number(Azumith) );
    if (s->contains("%el%")>0)
        s->replace( "%el%", QString::number(Elevation) );
    //qDebug() << "S made it here: " << *s;
}


