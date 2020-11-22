#include "confusingwordspair.h"

#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QVector>

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
        return;
    }

    QStringList list = str.split(",");

    if (list.size() != CWPAIR_ITEM_NUM)
    {
        return;
    }

    bool isTransOK = false;
    uint32_t temp;
    temp = list[CWPAIR_ITEM_CORR_TIMES].toUInt(&isTransOK);
    if (false == isTransOK)
    {
        return;
    }
    correctTimes = temp;

    w1.word = list[CWPAIR_ITEM_WORD_1];
    w1.exp = list[CWPAIR_ITEM_EXP_1];

    w2.word = list[CWPAIR_ITEM_WORD_2];
    w2.exp = list[CWPAIR_ITEM_WORD_2];
}

bool ConfusingWordsPair::saveConfusingWordsPairToFile(QString fileName, QVector<ConfusingWordsPair> *shownPairs, QVector<ConfusingWordsPair> *remainingPairs)
{
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

    foreach (ConfusingWordsPair wp, *shownPairs) {
        stm << wp.w1.word << "," << wp.w1.exp << ","
            << wp.w2.word << "," << wp.w2.exp << ","
            << wp.correctTimes;
    }

    foreach (ConfusingWordsPair wp, *remainingPairs) {
        stm << wp.w1.word << "," << wp.w1.exp << ","
            << wp.w2.word << "," << wp.w2.exp << ","
            << wp.correctTimes;
    }

    file.close();

    return true;
}
