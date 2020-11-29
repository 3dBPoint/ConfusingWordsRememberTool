#include "confusingwordspair.h"

#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QDebug>

#define CW_FILE_SEPARATOR ("\t")

enum CWPairStrItemIndex
{
    CWPAIR_ITEM_WORD_1 = 0,
    CWPAIR_ITEM_EXP_1,
    CWPAIR_ITEM_WORD_2,
    CWPAIR_ITEM_EXP_2,
    CWPAIR_ITEM_CORR_TIMES,

    CWPAIR_ITEM_NUM,
};

ConfusingWordsPair::ConfusingWordsPair(Word _w1, Word _w2, uint32_t _correctTimes):
    w1(_w1),
    w2(_w2),
    correctTimes(_correctTimes)
{

}

ConfusingWordsPair::ConfusingWordsPair(QString s)
{
    parse(s);
}

void ConfusingWordsPair::parse(QString str)
{
    if (str.isEmpty())
    {
        qDebug() << "empty string";
        return;
    }

    QStringList list = str.split(CW_FILE_SEPARATOR);

    if (list.size() != CWPAIR_ITEM_NUM)
    {
        qDebug() << "item num is" << list.size() << ", want" << CWPAIR_ITEM_NUM;
        foreach (QString s, list) {
            qDebug() << s;
        }
        return;
    }

    bool isTransOK = false;
    uint32_t temp;
    temp = list[CWPAIR_ITEM_CORR_TIMES].toUInt(&isTransOK);
    if (false == isTransOK)
    {
        qDebug() << "correct times trans failed";
        return;
    }
    correctTimes = temp;

    w1.word = list[CWPAIR_ITEM_WORD_1];
    w1.exp = list[CWPAIR_ITEM_EXP_1];

    w2.word = list[CWPAIR_ITEM_WORD_2];
    w2.exp = list[CWPAIR_ITEM_EXP_2];
}

bool ConfusingWordsPair::saveConfusingWordsPairToFile(QString fileName, QList<ConfusingWordsPair> &pairList)
{
    if (pairList.isEmpty())
    {
        return false;
    }

    if (fileName.isEmpty())
    {
        return false;
    }

    QFile file(fileName);

    if (true != file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream stm(&file);
    stm.setCodec("UTF-8");

    foreach (ConfusingWordsPair wp, pairList) {
        stm << wp.w1.word << CW_FILE_SEPARATOR << wp.w1.exp << CW_FILE_SEPARATOR
            << wp.w2.word << CW_FILE_SEPARATOR << wp.w2.exp << CW_FILE_SEPARATOR
            << wp.correctTimes << endl;
    }

    file.close();

    return true;
}
