#ifndef CWFILEMAKER_H
#define CWFILEMAKER_H

#include "confusingwordspair.h"

#include <QObject>

#define CW_FILE_MAKER_MAX_DIST 3

class CWFileMaker : public QObject
{
    Q_OBJECT
public:
    explicit CWFileMaker(QObject *parent = nullptr, QVector<Word> *pWVec = nullptr);

    void setPWordVec(QVector<Word> *value);
    void setMaxDist(const uint32_t &value);

signals:
    void cwFound(int dist, ConfusingWordsPair cwPair, CWFileMaker *sponsor);
    void cwFeedWordEaten(CWFileMaker *sponsor);
    void parsedOnce(CWFileMaker *sponsor); // every time when two words is checked, send this

public slots:
    void onFeedWord(Word w, unsigned int startSearchPos, CWFileMaker *sponsor);

private:
    uint32_t maxDist = CW_FILE_MAKER_MAX_DIST;
    QVector<Word> *pWordVec;
};

#endif // CWFILEMAKER_H
