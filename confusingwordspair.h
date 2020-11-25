#ifndef CONFUSINGWORDSPAIR_H
#define CONFUSINGWORDSPAIR_H

#include "word.h"

class ConfusingWordsPair
{
public:
    ConfusingWordsPair(Word _w1, Word _w2, uint32_t _correctTimes = 0);
    ConfusingWordsPair(QString s);
    ConfusingWordsPair() {}

    Word w1, w2;
    uint32_t correctTimes = 0;

    void parse(QString str);
    bool isEmpty()
    {
        return w1.isEmpty() || w2.isEmpty();
    }

    static bool saveConfusingWordsPairToFile(QString fileName, QVector<ConfusingWordsPair> *shownPairs, QVector<ConfusingWordsPair> *remainingPairs);
};

#endif // CONFUSINGWORDSPAIR_H
