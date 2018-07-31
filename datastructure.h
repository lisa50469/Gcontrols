#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H
#include <QString>

struct InitStruct
{
    InitStruct() {}
    QString str;
    InitStruct *next;
};

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
    //QString NetTermChars;
    InitStruct *Init;
    RespStruct *Resp;

    void AddStructComTerms (QString str);
    void AddStructInits (QString str);
    void AddStructResps (QString str1,QString str2);
    void PrintInits();
    void PrintResps();
    void GetResponseCode(QString resp, QString *code);
    QString GetComTermChars();
    QString GetNetTermChars();
};

#endif // DATASTRUCTURE_H
