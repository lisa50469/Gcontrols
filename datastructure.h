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
#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H
#include <QString>

struct RespStruct
{
    RespStruct() {}
    QString instr;
    QString outstr;
    RespStruct *next;
};

class DataStructure
{
public:
    DataStructure();
    QString ComTermChars;
    RespStruct *Resp;

    void AddStructComTerms (QString str);
    void AddStructResps (QString str1,QString str2);
    void PrintResps();
    void Clear();
    bool GetResponseCode(QString resp, QString *code);
    RespStruct *GetNextRespCode(RespStruct *p, QString *resp, QString *code);
    void InserVariablesInString(QString *s);
};

#endif // DATASTRUCTURE_H
