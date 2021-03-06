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
    this->setWindowTitle("Gcontrol by Lisa Nelson AK7WS Version: "+ version);

    QDirIterator Radioit(".", QStringList() << "*.rad", QDir::Files, QDirIterator::Subdirectories);
    while (Radioit.hasNext()) {
        Radioit.next();
        ui->comboBox_Radio_File_Config->addItem(Radioit.fileName());
    }

    QDirIterator Rotorit(".", QStringList() << "*.rot", QDir::Files, QDirIterator::Subdirectories);
    while (Rotorit.hasNext()) {
        Rotorit.next();
        ui->comboBox_Rotor_File_Config->addItem(Rotorit.fileName());
    }

    RadioSerialPort = new QSerialPort(this);
    RotorSerialPort = new QSerialPort(this);
    RadioServer = new QTcpServer;
    RotorServer = new QTcpServer;
    RadioCode = new DataStructure;
    RotorCode = new DataStructure;

    Azumith = 21;
    Elevation = 22;
    UplinkFreq = 145800000;
    DownlinkFreq = 446900000;
    MainWindow::DoRounding = false;

    // Now enable the NET server for the Radio
    RadioServer->listen(QHostAddress::Any,ui->lineEditRadioNetPort->text().toInt());
    connect(RadioServer,SIGNAL(newConnection()),this,SLOT(RadioNewNetConnection()));
    // Now enable the NET server for the Rotor
    RotorServer->listen(QHostAddress::Any,ui->lineEditRotorNetPort->text().toInt());
    connect(RotorServer,SIGNAL(newConnection()),this,SLOT(RotorNewNetConnection()));

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
    timer->start(500);
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
    QString s;

    if (RadioSerialPort->isOpen())
        {
        QString s = "EXIT"; // Send all the exit codes
        SendDataToCom(RadioCode, RadioSerialPort, &s, ui->plainTextEditRadio);
        RadioSerialPort->flush();
        RadioSerialPort->waitForBytesWritten(500);
        RadioSerialPort->close();
        ui->pushButton_open_radio_port->setText("Port Closed");
        ui->lineEdit_radio_port->setEnabled(0);
        }
    else
        {
        RadioSerialPort = new QSerialPort(this);
        RadioSerialPort->setPortName(ui->comboBox_radio_port->currentText());
        if (RadioSerialPort->open(QIODevice::ReadWrite))
            {
            if (RadioCode->GetResponseCode("BAUD",&s))
                RadioSerialPort->setBaudRate(s.toInt());
            else
                RadioSerialPort->setBaudRate(QSerialPort::Baud9600);
            RadioSerialPort->setDataBits(QSerialPort::Data8);
            RadioSerialPort->setFlowControl(QSerialPort::NoFlowControl);
            RadioSerialPort->setStopBits(QSerialPort::OneStop);

            RadioSerialPort->setDataTerminalReady(false);
            //RadioSerialPort->setDataTerminalReady(true);
            ui->pushButton_open_radio_port->setText("Port Open");
            ui->lineEdit_radio_port->setEnabled(1);
            QString s = "INIT"; // Send the data from the config file that was read.
            SendDataToCom(RadioCode, RadioSerialPort, &s, ui->plainTextEditRadio);
            }
        else
            {
            qDebug() << "Radio Com Port FAILED!";
            ui->lineEdit_radio_port->setEnabled(0);
            }
        }
}

//-----------------------------------------------------------------------
void MainWindow::on_pushButton_open_rotor_port_clicked()
{
    // Figure out how to set the focus to the rotor tab when clicked.
    //ui->tab_Rotor_COM->setVisible(1);
    ui->tabs->setFocus();
    ui->tab_Rotor_COM->setFocus();
    if (RotorSerialPort->isOpen())  // Port is already open
        {
        RotorSerialPort->close();
        ui->pushButton_open_rotor_port->setText("Port Closed");
        ui->lineEdit_rotor_port->setEnabled(0);
        }
    else    // Com port is not open.
        {
            RotorSerialPort->setPortName(ui->comboBox_rotor_port->currentText());
            if (RotorSerialPort->open(QIODevice::ReadWrite))
                {
                RotorSerialPort->setBaudRate(QSerialPort::Baud9600);
                RotorSerialPort->setDataBits(QSerialPort::Data8);
                RotorSerialPort->setFlowControl(QSerialPort::NoFlowControl);
                RotorSerialPort->setStopBits(QSerialPort::OneStop);
                ui->pushButton_open_rotor_port->setText("Port Open");
                ui->lineEdit_rotor_port->setEnabled(1);
                QString s = "INIT"; // Send the data from the config file that was read.
                SendDataToCom(RotorCode, RotorSerialPort, &s, ui->plainTextEditRotor);
                }
            else
                {
                qDebug() << "Rotor Com Port FAILED!";
                ui->lineEdit_rotor_port->setEnabled(0);
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
    RadioSerialPort->write(str.toUtf8());
    MainWindow::QStringReveal(str);
    if(ui->checkBox_logging->checkState())
        ui->plainTextEditRadio->appendPlainText(">: "+str);
}

//-----------------------------------------------------------------------
void MainWindow::on_lineEdit_rotor_port_returnPressed()
{
    //QByteArray data = ui->lineEdit_rotor_port->text().toLocal8Bit();
    //QString str = data;
    QString str =  ui->lineEdit_rotor_port->text();
   // data += '\n';   // Need to define termination in the config file too
    str += RotorCode->ComTermChars;
    RotorSerialPort->write(str.toUtf8());
    //str += '\n';
    MainWindow::QStringReveal(str);
    if(ui->checkBox_logging->checkState())
        ui->plainTextEditRotor->appendPlainText(">: "+str);
}

//-----------------------------------------------------------------------
void QSerialPort::errorOccurred(QSerialPort::SerialPortError error)
{
qDebug() << "Got a serial port ERROR!! " << error;
}

//-----------------------------------------------------------------------
QString MainWindow::GetComData(QSerialPort *whatcom,QPlainTextEdit *where)
{
    QString str;
    if (whatcom->isOpen())
        str = whatcom->readAll();

    if (str.length()>0)
        {
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
    if (RadioSocket->ConnectedState == QAbstractSocket::UnconnectedState)
        qDebug() << "Radio socket is not connected";

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
    connect(RotorSocket,SIGNAL(readyRead()),this,SLOT(RotorSocketReadyRead()));
}

//-----------------------------------------------------------------------
long MainWindow::RoundTo5k(long num)
{
    if (!MainWindow::DoRounding) return num;  // No rounding...

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
QString MainWindow::FreqBand(QString freq)
{
    int i = freq.toInt();
    if ((i > 420000000) && (i < 450000000)) return "70cm";
    if ((i > 222000000) && (i < 225000000)) return "1.25m";
    if ((i > 144000000) && (i < 148000000)) return "2m";
    if ((i > 50000000) && (i < 54000000)) return "6m";
    if ((i > 28000000) && (i < 29700000)) return "10m";
    if ((i > 14000000) && (i < 14350000)) return "20m";
    if ((i > 7000000) && (i < 7300000)) return "40m";
    if ((i > 3500000) && (i < 4000000)) return "80m";
    if ((i > 1800000) && (i < 2000000)) return "160m";
//qDebug() << "Frequency sent is: " << freq;
return "-";
}

//-----------------------------------------------------------------------
void MainWindow::RadioSocketReadyRead()
{
    QString s;
    QString NewUPFreq;
    QString NewDNFreq;
    char CommandString[50];
    char str[50];
    char freqformat[20];
    char c;
    int i;

    // Get the new data into standard string format.
    QByteArray data = RadioSocket->readAll();
    for (i=0;i<data.size();i++) CommandString[i] = data[i];
        CommandString[i] = '\0'; // Terminate the string

    RadioCode->GetResponseCode("FREQFORMAT",&s);
    for (i=0;i<s.toUtf8().size();i++)
        freqformat[i] = s.toUtf8()[i];
    freqformat[i] = '\0'; // Terminate the string

    i = data[0];    // First character of command.
    s = data;

    switch (i) {
    case 'S':
        s = "RPRT 0\r";
        RadioSocket->write(s.toUtf8());
        break;
    case 'I':
        sscanf(CommandString,"%c %ld",&c,&UplinkFreq);
        UplinkFreq = MainWindow::RoundTo5k(UplinkFreq);
        s = "RPRT 0\r";
        RadioSocket->write(s.toUtf8());
        sprintf(str,freqformat,UplinkFreq);
        UplinkFreqS = str;
        NewUPFreq = str;
        if (NewUPFreq != LastUplinkFreq && RadioSerialPort->isWritable()) // new freq
            {
            if (FreqBand(NewUPFreq) != FreqBand(LastUplinkFreq))
                {
                if (RadioCode->GetResponseCode("BANDUP",&NewUPFreq))
                    { // Fake the radio out so it can change bands
                    RadioSerialPort->write(NewUPFreq.toUtf8());
                    MainWindow::QStringReveal(NewUPFreq);
                    if(ui->checkBox_logging->checkState())
                        ui->plainTextEditRadio->appendPlainText(">: "+NewUPFreq);
                    }
                }
            LastUplinkFreq = str;
            RadioCode->GetResponseCode("SETUP",&s);
            InserVariablesInString(&s);
            s += RadioCode->ComTermChars;
            RadioSerialPort->write(s.toUtf8());
            MainWindow::QStringReveal(s);
            if(ui->checkBox_logging->checkState())
                ui->plainTextEditRadio->appendPlainText(">: "+s);
            }
        break;
    case 'F':
        sscanf(CommandString,"%c %ld",&c,&DownlinkFreq);
        s = "RPRT 0\r";
        RadioSocket->write(s.toUtf8());
        DownlinkFreq = MainWindow::RoundTo5k(DownlinkFreq);

        sprintf(str,freqformat,DownlinkFreq);
        DownlinkFreqS = str;
        NewDNFreq = str;
        if (NewDNFreq != LastDownlinkFreq && RadioSerialPort->isWritable()) // new freq
            {
            LastDownlinkFreq = str;
            RadioCode->GetResponseCode("SETDN",&s);
            InserVariablesInString(&s);
            s += RadioCode->ComTermChars;
            RadioSerialPort->write(s.toUtf8());
            MainWindow::QStringReveal(s);
            if(ui->checkBox_logging->checkState())
                ui->plainTextEditRadio->appendPlainText(">: "+s);
            }
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
        qDebug() << "Un-Processed Radio Command:" << data;
        s = "RPRT 0\r";
        RadioSocket->write(s.toUtf8());
        break;
    } // Done with the case statement.

}

//-----------------------------------------------------------------------
void MainWindow::RotorSocketReadyRead()
{
    QString s;
    char str[50];
    char az[50];
    char el[50];
    char c;
    char format[20];

    QByteArray data = RotorSocket->readAll();
    int i;

    RotorCode->GetResponseCode("FORMAT",&s);
    for (i=0;i<s.toUtf8().size();i++)
        format[i] = s.toUtf8()[i];
    format[i] = '\0'; // Terminate the string

    i = data[0];
    s = data;
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
        sprintf(str,format,Azumith,Elevation);
        s = str;
        s += RotorCode->ComTermChars;
        if(RotorSerialPort->isWritable())
        {
            RotorSerialPort->write(s.toUtf8());
            MainWindow::QStringReveal(s);
            if(ui->checkBox_logging->checkState())
                ui->plainTextEditRotor->appendPlainText(">: "+s);
        }
        break;
    case 'S':
        RotorSocket->write("RPRT 0\n");
        break;
    default:
        qDebug() << "Un-Processed Rotor Command:" << data;
        s = "RPRT 0\r";
        RotorSocket->write(s.toUtf8());
        break;
    }
}

//-----------------------------------------------------------------------
void MainWindow::on_pushButton_read_radio_config_clicked()
{
    QString cmd,s1,s2,line;
    QFile file(ui->comboBox_Radio_File_Config->currentText()); // XXX  Need to get the file name.
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         return;

     RadioCode->Clear();
     QTextStream in(&file);
     while  (!in.atEnd())
        {
            line = in.readLine();
            cmd = line.split(":").at(0);
            if (cmd == "TERM")
                {
                s1 = line.split(":").at(1);
                QChar c = s1.toInt();
                RadioCode->ComTermChars = c;
                }
            if (cmd == "ROUND")
                {
                DoRounding = true;
                }
            if (cmd == "RESP")
            {
                s1 = line.split(":").at(1);
                s2 = line.split(":").at(2);
                RadioCode->AddStructResps(s1,s2);
            }
            if (cmd == "INIT")
            {
                s1 = line.split(":").at(0);
                s2 = line.split(":").at(1);
                RadioCode->AddStructResps(s1,s2);
            }
            if (cmd == "EXIT")
            {
                s1 = line.split(":").at(0);
                s2 = line.split(":").at(1);
                RadioCode->AddStructResps(s1,s2);
            }
            if (cmd == "BANDDN")
            {
                s1 = line.split(":").at(0);
                s2 = line.split(":").at(1);
                RadioCode->AddStructResps(s1,s2);
            }
            if (cmd == "BANDUP")
            {
                s1 = line.split(":").at(0);
                s2 = line.split(":").at(1);
                RadioCode->AddStructResps(s1,s2);
            }
            if (cmd == "BAUD")
            {
                s1 = line.split(":").at(0);
                s2 = line.split(":").at(1);
                RadioCode->AddStructResps(s1,s2);
            }
        };
     ui->pushButton_open_radio_port->setEnabled(1);
}

//-----------------------------------------------------------------------
void MainWindow::SendDataToCom(DataStructure *cfg, QSerialPort *prt, QString *resp, QPlainTextEdit *where )
{
    RespStruct *pnext;
    QString code;
    pnext = cfg->Resp;  // Initially point to the beginning of the list.
    do
    {
//qDebug() << "Looing for resp:" << *resp;
        pnext = cfg->GetNextRespCode(pnext,resp, &code);
        if (code != "NULL")
        {
//qDebug() << "New test function got one:"<< code;
            InserVariablesInString(&code);
//qDebug() << "New2 test function got one:"<< code;
            code += cfg->ComTermChars;
            prt->write(code.toUtf8());
            MainWindow::QStringReveal(code);
            if(ui->checkBox_logging->checkState())
                where->appendPlainText(">: "+code);
        }
    }while (pnext != NULL);
}

//-----------------------------------------------------------------------
void MainWindow::on_pushButton_read_rotor_config_clicked()
{
    QString cmd,s1,s2,line;
    QFile file(ui->comboBox_Rotor_File_Config->currentText()); // XXX  Need to get the file name.
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         return;
    RotorCode->Clear();
    QTextStream in(&file);
    while  (!in.atEnd())
        {
            line = in.readLine();
//qDebug() << "config ---> " << line;
            cmd = line.split(":").at(0);
            if (cmd == "TERM")
                {
                s1 = line.split(":").at(1);
                QChar c = s1.toInt();
                RotorCode->ComTermChars = c;
                }
            if (cmd == "INIT")
            {
                s1 = line.split(":").at(0);
                s2 = line.split(":").at(1);
                RotorCode->AddStructResps(s1,s2);
            }
            if (cmd == "RESP")
            {
                s1 = line.split(":").at(1);
                s2 = line.split(":").at(2);
                RotorCode->AddStructResps(s1,s2);
            }
            if (cmd == "BAUD")
            {
                s1 = line.split(":").at(0);
                s2 = line.split(":").at(1);
                RotorCode->AddStructResps(s1,s2);
            }
        };
    ui->pushButton_open_rotor_port->setEnabled(1);
}

//-----------------------------------------------------------------------
void MainWindow::InserVariablesInString(QString *s)
{
    if (s->contains("%uplink%")>0)
        s->replace( "%uplink%", UplinkFreqS);
   // s->replace( "%uplink%", QString::number(UplinkFreq));
    if (s->contains("%downlink%")>0)
        s->replace( "%downlink%", DownlinkFreqS);
   // s->replace( "%downlink%", QString::number(DownlinkFreq));

    if (s->contains("%az%")>0)
        s->replace( "%az%", QString::number(Azumith));
    if (s->contains("%el%")>0)
        s->replace( "%el%", QString::number(Elevation));
}
