#include "cwfilemaker.h"
#include <QDebug>
#include <QThread>

CWFileMaker::CWFileMaker(QObject *parent, QVector<Word> *pWVec) :
    QObject(parent)
{
    pWordVec = pWVec;
}
static inline unsigned int min(unsigned int n1, unsigned int n2)
{
    return n1 < n2 ? n1 : n2;
}

static inline unsigned int max(unsigned int n1, unsigned int n2)
{
    return n1 > n2 ? n1 : n2;
}

static inline unsigned int min(unsigned int n1, unsigned int n2, unsigned int n3)
{
    uint32_t temp = min(n1, n2);
    return min(temp, n3);
}

static unsigned int editDistance(char *str1, unsigned int len1, char *str2, unsigned int len2, unsigned int curLen)
{
    if (curLen > CW_FILE_MAKER_MAX_DIST)
    {
        // the distance is long enough and do not need further recursive execuate
        return CW_FILE_MAKER_MAX_DIST + 1;
    }

    curLen += 1;

    if (0 == min(len1, len2))
    {
        return max(len1, len2);
    }

    return min(editDistance(str1 + 1, len1 - 1, str2, len2, curLen) + 1,
        editDistance(str1, len1, str2 + 1, len2 - 1, curLen) + 1,
        editDistance(str1 + 1, len1 - 1, str2 + 1, len2 - 1, curLen) + ((*str1 == *str2) ? 0 : 1));
}

void CWFileMaker::setPWordVec(QVector<Word> *value)
{
    pWordVec = value;
}

void CWFileMaker::onFeedWord(Word w, unsigned int startSearchPos, CWFileMaker *sponsor)
{
    if (this != sponsor)
    {
        return;
    }

    if (!pWordVec)
    {
        qCritical("null words vec");
        return;
    }

    if (startSearchPos >= (uint32_t)pWordVec->count())
    {
        qCritical("feed word start pos[%u] exceeds", startSearchPos);
        return;
    }

    uint32_t calcDist = 0;

    for (QVector<Word>::iterator it = pWordVec->begin() + startSearchPos; it != pWordVec->end(); it++)
    {
        calcDist = editDistance(w.word.toLatin1().data(), w.word.length(), it->word.toLatin1().data(), it->word.length(), 0);
        qDebug("[0x%08x] %u %s - %u %s (%d)",
               (uint32_t)this,
               startSearchPos,
               w.word.toLatin1().data(),
               it - pWordVec->begin(),
               it->word.toLatin1().data(),
               calcDist);

        emit parsedOnce(this);

        if (calcDist <= maxDist && calcDist > 0)
        {
            emit cwFound(calcDist, ConfusingWordsPair(w, *it, 0), this);
        }
    }

    emit cwFeedWordEaten(this);
}

void CWFileMaker::setMaxDist(const uint32_t &value)
{
    maxDist = value;
}
