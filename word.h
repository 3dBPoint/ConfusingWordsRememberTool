#ifndef WORD_H
#define WORD_H

#include <QString>

struct Word_explaination
{
    QString partOfSpeech;
    QString chinese;
};

class Word
{
public:
    Word();

    Word(QString wo, QString ex):
        word(wo),
        exp(ex)
    {}

    bool isEmpty()
    {
        return word.isEmpty() || exp.isEmpty();
    }

    QString word;
    QString exp;
//    QVector<Word_explaination> expVec;
};

#endif // WORD_H
