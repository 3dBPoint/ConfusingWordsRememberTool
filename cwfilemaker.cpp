#include "cwfilemaker.h"
#include <QDebug>
#include <QThread>

#define CW_FILE_MAKER_DEBUG                 0
#define CW_FILE_MAKER_DIST_ALGO_DEBUG       0

#define CW_FILE_MAKER_USE_ITERATE_ALGORITHM 1

CWFileMaker::CWFileMaker(QObject *parent, QVector<Word> *pWVec) :
    QObject(parent)
{
    pWordVec = pWVec;
#if CW_FILE_MAKER_DEBUG
    qDebug("[0x%08x] CW Maker constructed", (long)this);
#endif
}

CWFileMaker::~CWFileMaker()
{
#if CW_FILE_MAKER_DEBUG
    qDebug("[0x%08x] CW Maker destoried", (long)this);
#endif
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

unsigned int CWFileMaker::editDistance(char *str1, unsigned int len1, char *str2, unsigned int len2)
{
    // recrusive algorithm
    if (0 == min(len1, len2))
    {
        return max(len1, len2);
    }

    return min(editDistance(str1 + 1, len1 - 1, str2, len2) + 1,
        editDistance(str1, len1, str2 + 1, len2 - 1) + 1,
        editDistance(str1 + 1, len1 - 1, str2 + 1, len2 - 1) + ((*str1 == *str2) ? 0 : 1));
}

unsigned int CWFileMaker::editDistance(QString left, QString right)
{
    if (left.length() == 0 || right.length() == 0)
    {
        return max(left.length(), right.length());
    }

//    unsigned int *matrix = new unsigned int[(left.length() + 1) * (right.length() + 1)];
    QVector<QVector<uint32_t>> matrix;

    matrix.resize(right.length() + 1);
    for(int i = 0; i < matrix.size(); i++)
    {
        matrix[i].resize(left.length() + 1);
    }

    //first line
    int i = 0, j = 0;

    for (j = 0; j < left.length() + 1; j++)
    {
        matrix[0][j] = j;
    }

    for (i = 1; i < right.length() + 1; i++)
    {
        matrix[i][0] = i;

        for (j = 1; j < left.length() + 1; j++)
        {
            unsigned int temp = 1;

            if ((unsigned int)(j - 1) < min((unsigned int)left.length(), (unsigned int)right.length()))
            {
                if (left.at(j - 1) == right.at(j - 1))
                {
                    temp = 0;
                }
            }

            matrix[i][j] = min(matrix[i-1][j] + 1, matrix[i][j - 1] + 1, matrix[i - 1][j - 1] + temp);
        }
    }

#if CW_FILE_MAKER_DIST_ALGO_DEBUG
    QString debugStr;

    for (i = 0; i < right.length() + 1; i++)
    {
        debugStr.clear();
        for (j = 0; j < left.length() + 1; j++)
        {
            debugStr.append(QString::number(matrix[i][j]) + " ");
        }
        qDebug() << debugStr;
    }
#endif

    unsigned int result = matrix.last().last();

#if CW_FILE_MAKER_DIST_ALGO_DEBUG
    qDebug() << "result:" << result;
#endif

    return result;

//    QVector<unsigned int> prevDist(left.length() + 1);
//    QVector<unsigned int> *pPrevDist = &prevDist;

//    QVector<unsigned int> curDist(left.length() + 1);
//    QVector<unsigned int> *pCurDist = &curDist;

//    // init the first line dist
//    for (QVector<unsigned int>::iterator it = pPrevDist->begin(); it != pPrevDist->end(); it++)
//    {
//        *it = it - pPrevDist->begin();
//    }

//#if CW_FILE_MAKER_DIST_ALGO_DEBUG
//    QString debugStr;
//    foreach (unsigned int val, *pPrevDist)
//    {
//        debugStr.append(QString::number(val) + " ");
//    }
//    qDebug() << debugStr;
//#endif

//    for (int i = 0; i < right.length(); i++)
//    {
//        pCurDist->operator [](0) = i + 1;

//        for (int j = 1; j < pCurDist->size(); j++)
//        {
//            unsigned int temp = 1;

//            if (j < min((unsigned int)left.length(), (unsigned int)right.length()))
//            {
//                if (left.at(j) == right.at(j))
//                {
//                    temp = 0;
//                }
//            }

//            pCurDist->operator [](j) = min(pCurDist->at(j - 1) + 1,
//                    pPrevDist->at(j) + 1,
//                    pPrevDist->at(j - 1) + temp);
//        }

//        prevDist = curDist;

//#if CW_FILE_MAKER_DIST_ALGO_DEBUG
//        QString debugStr;
//        foreach (unsigned int val, *pPrevDist)
//        {
//            debugStr.append(QString::number(val) + " ");
//        }
//        qDebug() << debugStr;
//#endif
//    }

//    return pPrevDist->last();
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
#ifdef CW_FILE_MAKER_USE_ITERATE_ALGORITHM
        calcDist = editDistance(w.word, it->word);
#else
        calcDist = editDistance(w.word.toLatin1().data(), w.word.length(), it->word.toLatin1().data(), it->word.length());
#endif

#if CW_FILE_MAKER_DEBUG
        qDebug("[0x%08x] %u %s - %u %s (%d)",
               (uint32_t)this,
               startSearchPos,
               w.word.toLatin1().data(),
               it - pWordVec->begin() + 1,
               it->word.toLatin1().data(),
               calcDist);
#endif

        emit parsedOnce(this);

        if (calcDist > 0)
        {
            if (calcDist <= maxDist && calcDist * 2 <= min(w.word.length(), it->word.length()))
            {
                emit cwFound(calcDist, ConfusingWordsPair(w, *it, 0), this);
            }
        }
    }

    emit cwFeedWordEaten(this);
}

void CWFileMaker::setMaxDist(const uint32_t &value)
{
    maxDist = value;
}
