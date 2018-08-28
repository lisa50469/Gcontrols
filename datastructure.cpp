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
    Resp = NULL;
}

//-----------------------------------------------------------------------
void DataStructure::Clear()
    {
    ComTermChars = "";
    Resp = NULL;

    }

//-----------------------------------------------------------------------
void DataStructure::AddStructComTerms(QString str)
{
    ComTermChars = str;
}

//-----------------------------------------------------------------------
void DataStructure::AddStructResps(QString str1,QString str2)
{
    if (Resp == NULL)
    {
        Resp = new RespStruct;
        Resp->instr = str1;
        Resp->outstr = str2;
        Resp->next = NULL;
    }
    else
    {
        RespStruct *ptr = Resp;  // Start at the beginning.
        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }
        ptr->next = new RespStruct;
        ptr = ptr->next; // Now on the new resp box
        ptr->instr = str1;
        ptr->outstr = str2;
        ptr->next = NULL;
    }
}

//-----------------------------------------------------------------------
QString DataStructure::GetComTermChars()
{
    return ComTermChars;
}

//-----------------------------------------------------------------------
void DataStructure::PrintResps()
{
    RespStruct *ptr = Resp;  // Start at the beginning.
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
    RespStruct *ptr = Resp;  // Start at the beginning.
    while ((ptr != NULL) && (resp != ptr->instr))
    {
        if (!resp.indexOf(ptr->instr))
            {

            }
        else
        {
            ptr = ptr->next;
        }
    }
    *code = ptr->outstr;
}

//-----------------------------------------------------------------------
RespStruct *DataStructure::GetNextRespCode(RespStruct *p,QString *resp, QString *code)
{
    RespStruct *ptr = p;  // Start at the passed pointer.
    if (ptr == NULL)
    {
        *code = "NULL";
        return NULL; // Don't deal with NULL pointer!
    }
    else
    {
        while ((ptr != NULL) && (*resp != ptr->instr))
            ptr = ptr->next; // Search through list.

        if (ptr == NULL)
            {
                *code = "NULL";
                return NULL;  // Didn't find any more...
            }

        if (ptr->instr == *resp)
            {   // Got a match!
                *code = ptr->outstr;
                return ptr->next;   // Let calling function know where we left off.
            }
    }
return NULL;    // Make compiler happy with a return.
}


