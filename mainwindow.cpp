#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QTime>
#include <QDebug>

#define CONFUSING_WORD_OPERATE_TEXT_GEN "Generate!"
#define CONFUSING_WORD_OPERATE_TEXT_DEL "Delete!"

#define ORIGINAL_FILE_SEL_BTN_TEXT_SELECT "Browse"
#define ORIGINAL_FILE_SEL_BTN_TEXT_CLOSE  "Close"

#define CW_WORDS_FILE_DIR "/confusing_words/"

#define UI_CMBBOX_NO_CW_FILE_FOUND_TXT ("No file found")

#define CW_WORDS_MAX_DIST 3

#define FRONT_BODY "<html><head/><body><p align=\"center\"><span style=\" font-size:16pt;\">"
#define REAR_BODY "</span></p></body></html>"
#define CENTER_ALIGNMENT_STR(str) (FRONT_BODY + str + REAR_BODY)

/**** Debug Switch ****/
#define MAKE_CW_FILE_DEBUG 1
#define DISP_CW_PAIR_DEBUG 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pRemainCWPairs = new QList<ConfusingWordsPair>();
    pCitedCWPairs = new QList<ConfusingWordsPair>();

    RefreshFileList();

    QTime t;
    t = QTime::currentTime();
    qsrand(t.msec() + t.second() * 1000);

    connect(ui->ui_line_edit_origin_file_path, &QLineEdit::textChanged, this, [this](){ui->ui_btn_make_cw_file->setEnabled(true);});
}

MainWindow::~MainWindow()
{
    delete ui;
    if (pRemainCWPairs)
        delete pRemainCWPairs;
    if (pCitedCWPairs)
        delete pCitedCWPairs;
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

    QTime t;
    t.start();

    Word w;
    while (txtStream.readLineInto(&oneLineStr))
    {
        if (oneLineStr.isEmpty())
        {
            continue;
        }

        w = parseOriginalFileLine(oneLineStr);
        wordsVec.append(w);

        qDebug() << w.word << ":" << w.exp;
    }

    qDebug("read && parse file use %d ms, with %d items", t.elapsed(), wordsVec.size());

    if (QMessageBox::Ok == QMessageBox::question(this, QCoreApplication::applicationName(), "Make confusing words file now?", QMessageBox::Ok, QMessageBox::Cancel))
    {
        //TODO: use multi-thread to make cw file
        makeCWFiles();

        RefreshFileList();

        ui->ui_btn_make_cw_file->setEnabled(false);
    }

    originalFile.close();
    ui->ui_line_edit_origin_file_path->setText(originalFileName);
}

void MainWindow::on_ui_cmb_box_edit_distance_currentTextChanged(const QString &arg1)
{
#if 0
    if (arg1.isEmpty())
        return;

    QDir dir(QCoreApplication::applicationDirPath() + "/confusing_words/");
    if  (!dir.exists())
        dir.mkpath(QCoreApplication::applicationDirPath() + "/confusing_words/");

    confusingFileName = QCoreApplication::applicationDirPath() + QString("/confusing_words/%1.txt").arg(arg1);

    qDebug() << confusingFileName;

    QFileInfo finfo(confusingFileName);
    if (finfo.exists())
    {
        ui->ui_btn_operate_confusing_word_file->setText(CONFUSING_WORD_OPERATE_TEXT_DEL);
    }
    else
    {
        ui->ui_btn_operate_confusing_word_file->setText(CONFUSING_WORD_OPERATE_TEXT_GEN);
    }

    uint32_t randIndex = getShowPairRandomIndex();

    if (randIndex < 0)
    {
        return;
    }

    ConfusingWordsPair pair = pRemainCWPairs->at(randIndex);
    pRemainCWPairs->removeAt(randIndex);
    pShownCWPairs->append(pair);

    showOneCWPair(pair);
#endif
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
}

#if 0
bool MainWindow::makeCWFile(QString originalName, QString cwName, unsigned int dist)
{
    qDebug()<<"original file:"<<originalName;
    qDebug()<<"cw file:"<<cwName;
    qDebug()<<"dist:"<<dist;

    QFile originalFile(originalName);
    QFile cwFile(cwName);

    if (true != originalFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QCoreApplication::applicationName(), "original file open failed", QMessageBox::Ok);
        return false;
    }

    if (true != cwFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QCoreApplication::applicationName(), "cw file open failed", QMessageBox::Ok);
        originalFile.close();
        return false;
    }

    QTextStream origTxtStm(&originalFile);
    QTextStream compTxtStm(&originalFile);
    QTextStream cwTxtStm(&cwFile);

    origTxtStm.setCodec("UTF-8");
    compTxtStm.setCodec("UTF-8");
    cwTxtStm.setCodec("UTF-8");

    QString origLineStr, compLineStr;

    while (origTxtStm.readLineInto(&origLineStr))
    {
        qint64 prevOrigPos = origTxtStm.pos();
        compTxtStm.seek(prevOrigPos);

        while (compTxtStm.readLineInto(&compLineStr))
        {
            Word origWord = parseOriginalFileLine(origLineStr);
            Word compWord = parseOriginalFileLine(compLineStr);

            if (editDistance(origWord.word.toLatin1().data(), origWord.word.length(), compWord.word.toLatin1().data(), compWord.word.length()) == dist)
            {

                qDebug() << origWord.word << ":" << origWord.exp << "-" << compWord.word << ":" << compWord.exp;

                /* Header: cw_pair_1 cw_pair_1_exp cw_pair_2 cw_pair_2_exp OK_times*/
                cwTxtStm << origWord.word << ',' << origWord.exp << ','
                         << compWord.word << ',' << compWord.exp << ','
                         << '0';
            }
        }
        origTxtStm.seek(prevOrigPos);
    }

    originalFile.close();
    cwFile.close();
    return true;
}
#endif

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

    qDebug()<<"first space"<<firstSpace<<"2nd:"<<secondSpace;


    return Word(line.mid(firstSpace + 1, wordLen), line.mid(secondSpace + 1));
}

void MainWindow::showOneCWPairRandomly()
{
    int index = getShowPairRandomIndex();
    if (index < 0)
    {
        QMessageBox::warning(this, QCoreApplication::applicationName(), "No confusing words pair");
        return;
    }

    pCurrentCWPairs = pRemainCWPairs->at(index);

    pRemainCWPairs->removeAt(index);

    showOneCWPair(pCurrentCWPairs);
}

int32_t MainWindow::getShowPairRandomIndex()
{
    // TODO: if remain cw pairs is empty, swap it with the shown one

    // generate a random value between 0 to num of remaining confusing words
    if (pRemainCWPairs->isEmpty())
    {
        return -1;
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
        ui->ui_label_word_1->setText(CENTER_ALIGNMENT_STR(pair.w2.word));
        ui->ui_label_word_2->setText(CENTER_ALIGNMENT_STR(pair.w1.word));

        ui->ui_txt_edit_exp_1->setText(pair.w2.exp);
        ui->ui_txt_edit_exp_2->setText(pair.w1.exp);
    }

    //TODO: status bar info show
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

    for (int i = 1; i <= CW_WORDS_MAX_DIST; i ++)
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
    if (!pRemainCWPairs || !pCitedCWPairs)
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
    pCitedCWPairs->clear();

    while (tstm.readLineInto(&lineStr))
    {
        ConfusingWordsPair pair(lineStr);
        if (pair.isEmpty())
        {
            qDebug() << "read NG line: " << lineStr;
            continue;
        }
        pRemainCWPairs->append(pair);
    }

    if (pRemainCWPairs->isEmpty())
    {
        QMessageBox::critical(this, "Invalid CW file", "Cannot parse at least one line of " + fileName);
        return false;
    }

    return true;
}

void MainWindow::on_ui_btn_next_pair_clicked()
{
    showOneCWPairRandomly();
}

void MainWindow::on_ui_btn_load_cw_file_clicked()
{
    QString s = ui->ui_cmb_box_edit_distance->currentText();

    if (s == UI_CMBBOX_NO_CW_FILE_FOUND_TXT)
    {
        return;
    }

    int dist = s.toInt();

    if (true != loadCWFile(cwFileName(dist)))
    {
        return;
    }

    showOneCWPairRandomly();
}

void MainWindow::on_ui_btn_make_cw_file_clicked()
{
    if (ui->ui_line_edit_origin_file_path->text().isEmpty())
    {
        return;
    }

    makeCWFiles();

    RefreshFileList();

    ui->ui_btn_make_cw_file->setEnabled(false);
}
