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
#include "datastructure.h"
#include "mainwindow.h"
#include <QString>
#include <QDebug>

DataStructure::DataStructure()
{
    ComTermChars = "";
    //NetTermChars = "";
    Init = NULL;
    Resp = NULL;
}

//-----------------------------------------------------------------------
void DataStructure::Clear()
{
    if (Init != NULL)
    {
    Init = NULL;
    Resp = NULL;
    }
}

//-----------------------------------------------------------------------
void DataStructure::AddStructComTerms(QString str)
{
    ComTermChars = str;
}

//-----------------------------------------------------------------------
void DataStructure::AddStructInits(QString str)
{
    if (Init == NULL)
    {
        //qDebug() << "First box is empty!";
        Init = new InitStruct;
        Init->str = str;
        Init->next = NULL;
    }
    else
    {
        //qDebug() << "First box is NOT empty!";
        InitStruct *ptr = Init;  // Start at the beginning.
        while (ptr->next != NULL)
        {
            //qDebug() << "--->" << ptr->str;
            ptr = ptr->next;
        }
        ptr->next = new InitStruct;
        ptr = ptr->next; // Now on the new box
        ptr->str = str;
        ptr->next = NULL;

    }
}

//-----------------------------------------------------------------------
void DataStructure::PrintInits()
{
    InitStruct *ptr = Init;  // Start at the beginning.
    qDebug() << "------- Init List ----------";
    qDebug() << "Com Termination Characters: " << ComTermChars;
    while (ptr != NULL)
    {
        qDebug() << "--->" << ptr->str;
        ptr = ptr->next;
    }
}

//-----------------------------------------------------------------------
void DataStructure::AddStructResps(QString str1,QString str2)
{
    if (Resp == NULL)
    {
        //qDebug() << "First box is empty!";
        Resp = new RespStruct;
        Resp->instr = str1;
        Resp->outstr = str2;
        Resp->next = NULL;
    }
    else
    {
        //qDebug() << "First resp box is NOT empty!";
        RespStruct *ptr = Resp;  // Start at the beginning.
        while (ptr->next != NULL)
        {
            //qDebug() << "--->" << ptr->str;
            ptr = ptr->next;
        }
        ptr->next = new RespStruct;
        ptr = ptr->next; // Now on the new resp box
        //ptr->str = str;
        ptr->instr = str1;
        ptr->outstr = str2;
        ptr->next = NULL;

    }
}

//-----------------------------------------------------------------------
QString DataStructure::GetComTermChars()
{
    //qDebug() << "Termination Characters: " << TermChars;
    return ComTermChars;
}

//-----------------------------------------------------------------------
void DataStructure::PrintResps()
{
    RespStruct *ptr = Resp;  // Start at the beginning.
    //qDebug() << ptr->RespStruct.
    qDebug() << "------- Response List ----------";
    while (ptr != NULL)
    {
        qDebug() << "--->" << ptr->instr << "   --->" << ptr->outstr;
        ptr = ptr->next;
    }
}

//-----------------------------------------------------------------------
void DataStructure::GetResponseCode(QString resp, QString *code)
{
    QString s;
    //qDebug() <<  "Working on response code...";
    //qDebug() << "Searching for:" << resp;
    RespStruct *ptr = Resp;  // Start at the beginning.
    while ((ptr != NULL) && (resp != ptr->instr))
    {
       // qDebug() << "-->" << ptr->instr << ptr->outstr;
        if (!resp.indexOf(ptr->instr))
            {
            //qDebug() << "Found the response!!" << ptr->outstr;

            //ptr->outstr
            //return ptr->outstr;
            }
        else
        {
            //qDebug() << "NOT a match:" << ptr->instr;
            ptr = ptr->next;
        }
    }
    *code = ptr->outstr;
    //return NULL;
}

