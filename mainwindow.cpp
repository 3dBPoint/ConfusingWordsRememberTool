#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QThread>

#define CW_WORDS_FILE_DIR "/confusing_words/"

#define UI_CMBBOX_NO_CW_FILE_FOUND_TXT ("No file found")

#define FRONT_BODY "<html><head/><body><p align=\"center\"><span style=\" font-size:16pt;\">"
#define REAR_BODY "</span></p></body></html>"
#define CENTER_ALIGNMENT_STR(str) (FRONT_BODY + str + REAR_BODY)

#define CW_MAKER_THREAD_NUM             (4)
#define CW_REMEMBER_MAX_CORRECT_TIMES   (5)

/**** Debug Switch ****/
#define MAKE_CW_FILE_DEBUG 0
#define DISP_CW_PAIR_DEBUG 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pRemainCWPairs = new QList<ConfusingWordsPair>();
    pCitedCWPairs = new QList<ConfusingWordsPair>();
    pShownCWPairs = new QList<ConfusingWordsPair>();

    RefreshFileList();

    QTime t;
    t = QTime::currentTime();
    qsrand(t.msec() + t.second() * 1000);

    qRegisterMetaType<Word>("Word");
    qRegisterMetaType<ConfusingWordsPair>("ConfusingWordsPair");
}

MainWindow::~MainWindow()
{
    // Save file first if there is any
    if (selctedDist > 0)
    {
        if (true != ConfusingWordsPair::saveConfusingWordsPairToFile(cwFileName(selctedDist), (*pCitedCWPairs + *pRemainCWPairs + *pShownCWPairs)))
        {
            QMessageBox::critical(this, "critical", "Cannot save confusing words in use to file.", QMessageBox::Yes, QMessageBox::No);
        }
    }

    delete ui;

    if (pRemainCWPairs)
        delete pRemainCWPairs;

    if (pCitedCWPairs)
        delete pCitedCWPairs;

    if (pShownCWPairs)
        delete pShownCWPairs;

//    if (progWidget && progWidgetThread)
//    {
//        progWidgetThread->quit();
//        progWidgetThread->wait();
//    }

//    progWidget = nullptr;
//    progWidgetThread = nullptr;
    if (progWidget)
    {
        delete progWidget;
        progWidget = nullptr;
    }

    if (cwFileMakerThreads.isEmpty())
    {
        for (QMap<CWFileMaker *, QThread *>::iterator it = cwFileMakerThreads.begin(); it != cwFileMakerThreads.end(); it++)
        {
            it.value()->quit();
            it.value()->wait();

            cwFileMakerThreads.remove(it.key());
        }
    }
}

void MainWindow::onCWFound(int dist, ConfusingWordsPair cwPair)
{
    if (cwPair.isEmpty())
    {
        return;
    }

    QFile file(cwFileName(dist));

    if (true != file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        QMessageBox::warning(this, QCoreApplication::applicationName(), file.fileName() + "open failed", QMessageBox::Ok);
        return;
    }

    QTextStream stm(&file);
    stm.setCodec("UTF-8");

#if MAKE_CW_FILE_DEBUG
    qDebug() <<"find one:" << "dist=" << dist << cwPair.w1.word << cwPair.w1.exp << "-" << cwPair.w2.word << cwPair.w2.exp;
#endif

    /* Header: cw_pair_1 cw_pair_1_exp cw_pair_2 cw_pair_2_exp OK_times*/
    stm << cwPair.w1.word << '\t' << cwPair.w1.exp << '\t'
              << cwPair.w2.word << '\t' << cwPair.w2.exp << '\t'
              << '0' << endl;

    file.close();
}

void MainWindow::onCWFeedWordEaten(CWFileMaker *sponsor)
{
    if (wordsVecIt != wordsVec.end() - 1)
    {
        emit feedWord(*wordsVecIt, wordsVecIt - wordsVec.begin() + 1, sponsor);
        wordsVecIt++;
    }
    else
    {
        QThread *thread = cwFileMakerThreads.value(sponsor);
        thread->quit();
        thread->wait();

        qDebug("[0x%08x] finished", sponsor);

        if (0 == cwFileMakerThreads.remove(sponsor))
        {
            QMessageBox::warning(this, "Remove thread failed", QString("%1's thread remove failed").arg((uint32_t)sponsor));
            return;
        }
    }

    if (cwFileMakerThreads.isEmpty())
    {
        qDebug() << "make cw files use" << cwMakeTimer.elapsed() << "ms";

        RefreshFileList();

//        if (progWidget && progWidgetThread)
//        {
//            progWidgetThread->quit();
//            progWidgetThread->wait();
//        }

        if (progWidget)
        {
            delete progWidget;
            progWidget = nullptr;
        }
//        progWidgetThread = nullptr;

        QMessageBox::information(this, "make cw files", "make complete");
    }
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

static unsigned int editDistance(char *str1, unsigned int len1, char *str2, unsigned int len2)
{
    if (0 == min(len1, len2))
    {
        return max(len1, len2);
    }

    return min(editDistance(str1 + 1, len1 - 1, str2, len2) + 1,
        editDistance(str1, len1, str2 + 1, len2 - 1) + 1,
        editDistance(str1 + 1, len1 - 1, str2 + 1, len2 - 1) + ((*str1 == *str2) ? 0 : 1));
}

void MainWindow::on_ui_btn_select_origin_file_clicked()
{
    QString originalFileName;

    originalFileName = QFileDialog::getOpenFileName(this,
                                                    QCoreApplication::applicationName(),
                                                    QCoreApplication::applicationFilePath(),
                                                    "text(*.txt)");
    if (originalFileName.isEmpty())
    {
        return;
    }

    QFile originalFile(originalFileName);

    if (true != originalFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QCoreApplication::applicationName(), "file open failed", QMessageBox::Ok);
        return;
    }

    QTextStream txtStream(&originalFile);
    txtStream.setCodec("UTF-8");

    QString oneLineStr;
    Word w;

    QTime t;
    t.start();

    wordsVec.clear();
    while (txtStream.readLineInto(&oneLineStr))
    {
        if (oneLineStr.isEmpty())
        {
            continue;
        }

        w = parseOriginalFileLine(oneLineStr);

        if (w.isEmpty())
        {
            continue;
        }

        wordsVec.append(w);
    }

    qDebug("read && parse file use %d ms, with %d items", t.elapsed(), wordsVec.size());
    qDebug("going to calculate %u times", getCWPairCalcTimes());

    originalFile.close();
    ui->ui_line_edit_origin_file_path->setText(originalFileName);

    if (QMessageBox::Ok == QMessageBox::question(this, QCoreApplication::applicationName(), "Make confusing words file now?", QMessageBox::Ok, QMessageBox::Cancel))
    {
        makeCWFiles();

        ui->ui_btn_make_cw_file->setEnabled(false);
    }
    else
    {
        ui->ui_btn_make_cw_file->setEnabled(true);
    }
}

#if 0
void MainWindow::on_ui_btn_operate_confusing_word_file_clicked()
{
    if (ui->ui_btn_operate_confusing_word_file->text() == CONFUSING_WORD_OPERATE_TEXT_DEL)
    {
        if (false == QFile::remove(confusingFileName))
        {
            QMessageBox::critical(this, "critical", "remove failed!", QMessageBox::Ok);
            return;
        }

        QMessageBox::information(this, "info", "remove success", QMessageBox::Ok);
        ui->ui_btn_operate_confusing_word_file->setText(CONFUSING_WORD_OPERATE_TEXT_GEN);
    }
    else if (ui->ui_btn_operate_confusing_word_file->text() == CONFUSING_WORD_OPERATE_TEXT_GEN)
    {
        if (pMsgBox)
            delete pMsgBox;
        pMsgBox = new QMessageBox(this);

        pMsgBox->setText("Creating File, and will take a while, please wait...");
        pMsgBox->setModal(false);
        pMsgBox->show();

        if (!makeCWFile(confusingFileName, ui->ui_cmb_box_edit_distance->currentText().toUInt()))
        {
            QMessageBox::critical(this, "critical", "Generate Failed!");
            return;
        }
        ui->ui_btn_operate_confusing_word_file->setText(CONFUSING_WORD_OPERATE_TEXT_DEL);
    }
    else
    {
        QMessageBox::critical(this, "critical", "unknown op", QMessageBox::Ok);
    }
}
#endif

bool MainWindow::makeCWFiles()
{
#if 0
    if (wordsVec.isEmpty())
    {
        return false;
    }

    QFile cwFiles[CW_WORDS_MAX_DIST];
    QTextStream cwTxtStms[CW_WORDS_MAX_DIST];
    QString cwName;

    for (int i = 0; i < CW_WORDS_MAX_DIST; i++)
    {
        cwName = cwFileName(i + 1);
        cwFiles[i].setFileName(cwName);

        if (true != cwFiles[i].open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, QCoreApplication::applicationName(), cwName + "open failed", QMessageBox::Ok);
            return false;
        }

        cwTxtStms[i].setDevice(&cwFiles[i]);
        cwTxtStms[i].setCodec("UTF-8");
    }

    QTime t;
    t.start();

    uint32_t calcDist = 0;

    for (QVector<Word>::iterator it = wordsVec.begin(); it != wordsVec.end(); it++)
    {
        qDebug() << "word1:" << it - wordsVec.begin() << it->word;

        for (QVector<Word>::iterator ir = it + 1; ir != wordsVec.end(); ir++)
        {
            qDebug() << "word2" << ir - wordsVec.begin() << ir->word;

            calcDist = editDistance(it->word.toLatin1().data(), it->word.length(), ir->word.toLatin1().data(), ir->word.length());

            if (calcDist <= CW_WORDS_MAX_DIST && calcDist > 0)
            {
                qDebug() <<"find one: " << it->word << ":" << it->exp << "-" << ir->word << ":" << ir->exp << "dist = " << calcDist;

                /* Header: cw_pair_1 cw_pair_1_exp cw_pair_2 cw_pair_2_exp OK_times*/
                cwTxtStms[calcDist - 1] << it->word << '\t' << it->exp << '\t'
                         << ir->word << '\t' << ir->exp << '\t'
                         << '0' << endl;
            }
        }
    }

    qDebug("make cw file use %d ms", t.elapsed());

    for (int i = 0; i < CW_WORDS_MAX_DIST; i++)
    {
        cwFiles[i].close();
    }

    return true;
#endif
    if (wordsVec.isEmpty())
    {
        return false;
    }

    if (!cwFileMakerThreads.isEmpty())
    {
        for (QMap<CWFileMaker *, QThread *>::iterator it = cwFileMakerThreads.begin(); it != cwFileMakerThreads.end(); it++)
        {
            it.value()->quit();
            it.value()->wait();
        }

        cwFileMakerThreads.clear();
    }

    QThread *cwMakeThread;
    CWFileMaker *cwFileMaker;

    wordsVecIt = wordsVec.begin();

    // new and start progress display widget

    progWidget = new CWFileMakeProgressWidget();
//    progWidgetThread = new QThread;

//    progWidget->moveToThread(progWidgetThread);

    progWidget->setWindowTitle("Progress");
    progWidget->setWindowFlags(windowFlags() &~ Qt::WindowCloseButtonHint);

    progWidget->onSetFull(getCWPairCalcTimes());

//    connect(progWidgetThread, &QThread::finished, progWidgetThread, &QThread::deleteLater);
//    connect(progWidgetThread, &QThread::finished, progWidget, &CWFileMakeProgressWidget::deleteLater);

//    progWidgetThread->start();

    progWidget->show();

    //delete files
    for (int i = 0; i < CW_FILE_MAKER_MAX_DIST; i++)
    {
        QFile::remove(cwFileName(i));
    }

    // new and start cw maker threads
    for (int i = 0; i < CW_MAKER_THREAD_NUM; i++)
    {
        cwFileMaker = new CWFileMaker(nullptr, &wordsVec);

        cwMakeThread = new QThread;

        cwFileMakerThreads.insert(cwFileMaker, cwMakeThread);

        cwFileMaker->setMaxDist(CW_FILE_MAKER_MAX_DIST);
        cwFileMaker->moveToThread(cwMakeThread);

        connect(cwMakeThread, &QThread::finished, cwFileMaker, &CWFileMaker::deleteLater);
        connect(cwMakeThread, &QThread::finished, cwMakeThread, &CWFileMaker::deleteLater);

        connect(cwFileMaker, &CWFileMaker::cwFound, this, &MainWindow::onCWFound);
        connect(cwFileMaker, &CWFileMaker::cwFeedWordEaten, this, &MainWindow::onCWFeedWordEaten);

        connect(cwFileMaker, &CWFileMaker::parsedOnce, progWidget, &CWFileMakeProgressWidget::onProgressIncreaseOnece);

        connect(this, &MainWindow::feedWord, cwFileMaker, &CWFileMaker::onFeedWord);

        cwMakeThread->start();

        //start feed words to the thread created
        onCWFeedWordEaten(cwFileMaker);
    }

    cwMakeTimer.start();

    cwFullCalculateTimes = getCWPairCalcTimes();

    return true;
}

Word MainWindow::parseOriginalFileLine(QString &line)
{
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);
    int wordLen = secondSpace - firstSpace - 1;

    if (-1 == firstSpace || -1 == secondSpace)
    {
        qDebug() << "NG, str:" << line;
        return Word();
    }

    return Word(line.mid(firstSpace + 1, wordLen), line.mid(secondSpace + 1));
}

uint32_t MainWindow::getCWPairCalcTimes()
{
    int32_t wordsNum = wordsVec.count();

    return (wordsNum * (wordsNum - 1)) / 2;
}

void MainWindow::showOneCWPairRandomly()
{
    ConfusingWordsPair currentCWPair;

    currentShowWordIndex = getShowPairRandomIndex();
    if (currentShowWordIndex < 0)
    {
        QMessageBox::warning(this, QCoreApplication::applicationName(), "No confusing words pair");
        return;
    }

    currentCWPair = pRemainCWPairs->at(currentShowWordIndex);

    if (currentCWPair.correctTimes >= CW_REMEMBER_MAX_CORRECT_TIMES)
    {
        qDebug() << "impossible correct times";
        return;
    }

    double progress = (double)pCitedCWPairs->count() / (pCitedCWPairs->count() + pRemainCWPairs->count() + pShownCWPairs->count());
    QString message = statusBarStudyProgressStr(pRemainCWPairs->count(), pShownCWPairs->count(), pCitedCWPairs->count(), progress);
    ui->statusBar->showMessage(message);

    showOneCWPair(currentCWPair);
    qDebug() << "correct times:" << currentCWPair.correctTimes;
}

int32_t MainWindow::getShowPairRandomIndex()
{
    // generate a random value between 0 to num of remaining confusing words
    if (pRemainCWPairs->isEmpty())
    {
        if (pShownCWPairs->isEmpty())
        {
            return -1;
        }
        else
        {
            QList<ConfusingWordsPair> *temp = pRemainCWPairs;
            pRemainCWPairs = pShownCWPairs;
            pShownCWPairs = temp;

            pShownCWPairs->clear();

            qDebug() << "swap cited list and remain list";
        }
    }

    return qrand() % pRemainCWPairs->size();
}

void MainWindow::showOneCWPair(ConfusingWordsPair pair)
{
    bool isReverse = qrand() % 2; // 0 or 1

    if (false == isReverse)
    {
        ui->ui_label_word_1->setText(CENTER_ALIGNMENT_STR(pair.w1.word));
        ui->ui_label_word_2->setText(CENTER_ALIGNMENT_STR(pair.w2.word));

        ui->ui_txt_edit_exp_1->setText(pair.w1.exp);
        ui->ui_txt_edit_exp_2->setText(pair.w2.exp);
    }
    else
    {
        qDebug() << "reversed display";

        ui->ui_label_word_1->setText(CENTER_ALIGNMENT_STR(pair.w2.word));
        ui->ui_label_word_2->setText(CENTER_ALIGNMENT_STR(pair.w1.word));

        ui->ui_txt_edit_exp_1->setText(pair.w2.exp);
        ui->ui_txt_edit_exp_2->setText(pair.w1.exp);
    }

    ui->ui_txt_edit_exp_1->hide();
    ui->ui_txt_edit_exp_2->hide();
}

void MainWindow::RefreshFileList()
{
    QDir dir(QCoreApplication::applicationDirPath() + CW_WORDS_FILE_DIR);

    ui->ui_cmb_box_edit_distance->clear();

    if  (!dir.exists())
    {
        ui->ui_cmb_box_edit_distance->setCurrentText(UI_CMBBOX_NO_CW_FILE_FOUND_TXT);

        dir.mkpath(QCoreApplication::applicationDirPath() + CW_WORDS_FILE_DIR);

        return;
    }

    for (int i = 1; i <= CW_FILE_MAKER_MAX_DIST; i ++)
    {
        QString confusingFileName = cwFileName(i);

        QFileInfo finfo(confusingFileName);

        if (!finfo.exists())
        {
            qDebug() << confusingFileName << "not exists";
            continue;
        }
        qDebug() << confusingFileName << "exists";

        ui->ui_cmb_box_edit_distance->addItem(QString::number(i));
    }

    if (0 == ui->ui_cmb_box_edit_distance->count())
    {
        ui->ui_cmb_box_edit_distance->setCurrentText(UI_CMBBOX_NO_CW_FILE_FOUND_TXT);
    }
}

QString MainWindow::cwFileName(int dist)
{
    return QCoreApplication::applicationDirPath() + CW_WORDS_FILE_DIR + QString("%1.txt").arg(dist);
}

bool MainWindow::loadCWFile(QString fileName)
{
    if (!pRemainCWPairs || !pCitedCWPairs || !pShownCWPairs)
    {
        QMessageBox::critical(this, "internal error", (__FILE__ + __LINE__));
        return false;
    }

    QFile file(fileName);

    if (true != file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << fileName << "open failed";
        return false;
    }

    QTextStream tstm(&file);
    QString lineStr;

    // TODO: save current CW pairs progress
    tstm.setCodec("UTF-8");

    pRemainCWPairs->clear();
    pShownCWPairs->clear();
    pCitedCWPairs->clear();

    while (tstm.readLineInto(&lineStr))
    {
        ConfusingWordsPair pair(lineStr);
        if (pair.isEmpty())
        {
            qDebug() << "read NG line: " << lineStr;
            continue;
        }

        if (pair.correctTimes < CW_REMEMBER_MAX_CORRECT_TIMES)
        {
            pRemainCWPairs->append(pair);
        }
        else
        {
            pCitedCWPairs->append(pair);
        }
    }

    if (pRemainCWPairs->isEmpty())
    {
        QMessageBox::critical(this, "Invalid CW file", "Invalid file or all remembered?\nFile: " + fileName);

        return false;
    }

    return true;
}

QString MainWindow::statusBarStudyProgressStr(int remain, int shown, int recited, double prog)
{
    return QString("remain[%1]  shown[%2]  recited[%3]  (%4%)").arg(remain).arg(shown).arg(recited).arg(prog * 100, 0, 'g', 3);
}

void MainWindow::displayCurrentPairExp(bool left, bool right)
{
    if (currentShowWordIndex < 0)
    {
        return;
    }

    if (left)
        ui->ui_txt_edit_exp_1->show();

    if (right)
        ui->ui_txt_edit_exp_2->show();
}

void MainWindow::on_ui_btn_next_pair_clicked()
{
    showOneCWPairRandomly();
}

void MainWindow::on_ui_btn_load_cw_file_clicked()
{
    // Save file first if there is any
    if (selctedDist > 0)
    {
        if (true != ConfusingWordsPair::saveConfusingWordsPairToFile(cwFileName(selctedDist), (*pCitedCWPairs + *pRemainCWPairs + *pShownCWPairs)))
        {
            int32_t ret = QMessageBox::critical(this, "critical", "Cannot save confusing words in use to file, discard?", QMessageBox::Ok, QMessageBox::No);
            if (QMessageBox::Ok != ret)
            {
                return;
            }
        }
    }

    selctedDist = -1;
    QString s = ui->ui_cmb_box_edit_distance->currentText();

    if (s == UI_CMBBOX_NO_CW_FILE_FOUND_TXT)
    {
        return;
    }

    selctedDist = s.toInt();

    if (true != loadCWFile(cwFileName(selctedDist)))
    {
        pRemainCWPairs->clear();
        pShownCWPairs->clear();
        pCitedCWPairs->clear();

        QMessageBox::critical(this, "critical", "open cw file failed" + cwFileName(selctedDist));

        return;
    }

    qDebug() << "This cw file has" << pRemainCWPairs->count() << "item";

    showOneCWPairRandomly();
}

void MainWindow::on_ui_btn_make_cw_file_clicked()
{
    if (ui->ui_line_edit_origin_file_path->text().isEmpty())
    {
        return;
    }

    makeCWFiles();

    ui->ui_btn_make_cw_file->setEnabled(false);
}

void MainWindow::on_ui_btn_remove_the_pair_clicked()
{
    if (currentShowWordIndex < 0)
    {
        return;
    }

    ConfusingWordsPair pair = pRemainCWPairs->at(currentShowWordIndex);
    pair.correctTimes = CW_REMEMBER_MAX_CORRECT_TIMES;
    pCitedCWPairs->append(pair);

    pRemainCWPairs->removeAt(currentShowWordIndex);

    showOneCWPairRandomly();
}

void MainWindow::on_ui_btn_remembered_the_pair_once_clicked()
{
    if (currentShowWordIndex < 0)
    {
        return;
    }

    ConfusingWordsPair pair = pRemainCWPairs->at(currentShowWordIndex);

    pair.correctTimes += 1;

    if (pair.correctTimes < CW_REMEMBER_MAX_CORRECT_TIMES)
    {
        pShownCWPairs->append(pair);
    }
    else
    {
        pCitedCWPairs->append(pair);
    }

    pRemainCWPairs->removeAt(currentShowWordIndex);

    showOneCWPairRandomly();
}

void MainWindow::on_ui_btn_forget_the_pair_clicked()
{
    if (currentShowWordIndex < 0)
    {
        return;
    }

    (*pRemainCWPairs)[currentShowWordIndex].correctTimes = 0;

    showOneCWPairRandomly();
}

void MainWindow::on_ui_btn_show_meaning_clicked()
{
    displayCurrentPairExp(true, true);
}
