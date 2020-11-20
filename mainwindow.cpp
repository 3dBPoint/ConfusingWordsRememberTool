#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

#define CONFUSING_WORD_OPERATE_TEXT_GEN "Generate!"
#define CONFUSING_WORD_OPERATE_TEXT_DEL "Delete!"

#define ORIGINAL_FILE_SEL_BTN_TEXT_SELECT "Browse"
#define ORIGINAL_FILE_SEL_BTN_TEXT_CLOSE  "Close"

#define FRONT_BODY "<html><head/><body><p align=\"center\"><span style=\" font-size:36pt;\">"
#define REAR_BODY "</span></p></body></html>"

/**** Debug Switch ****/
#define MAKE_CW_FILE_DEBUG 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->ui_cmb_box_edit_distance->addItems(QStringList() << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10");
}

MainWindow::~MainWindow()
{
    delete ui;
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
//    QString originalFileName;

    originalFileName = QFileDialog::getOpenFileName(this,
                                                    QCoreApplication::applicationName(),
                                                    QCoreApplication::applicationFilePath(),
                                                    "text(*.txt)");
    if (originalFileName.isEmpty())
    {
        return;
    }

//    originalFileStr = originalFile.readAll();

//    if (originalFileStr.isEmpty())
//    {
//        QMessageBox::warning(this, QCoreApplication::applicationName(), "empty original file", QMessageBox::Ok);
//        originalFile.close();
//        return;
//    }

    ui->ui_line_edit_origin_file_path->setText(originalFileName);
}

void MainWindow::on_ui_cmb_box_edit_distance_currentTextChanged(const QString &arg1)
{
    if (arg1.isEmpty())
        return;

    QDir dir(QCoreApplication::applicationDirPath() + "/confusing_words/");
    if  (!dir.exists())
        dir.mkpath(QCoreApplication::applicationDirPath() + "/confusing_words/");

    confusingFileName = QCoreApplication::applicationDirPath() + QString("/confusing_words/%1.txt").arg(arg1);

#if MAKE_CW_FILE_DEBUG
    qDebug() << confusingFileName;
#endif

    QFileInfo finfo(confusingFileName);
    if (finfo.exists())
    {
        ui->ui_btn_operate_confusing_word_file->setText(CONFUSING_WORD_OPERATE_TEXT_DEL);
    }
    else
    {
        ui->ui_btn_operate_confusing_word_file->setText(CONFUSING_WORD_OPERATE_TEXT_GEN);
    }
}

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
        if (!makeCWFile(originalFileName, confusingFileName, ui->ui_cmb_box_edit_distance->currentText().toUInt()))
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
#if MAKE_CW_FILE_DEBUG
                qDebug() << origWord.word << ":" << origWord.exp << "-" << compWord.word << ":" << compWord.exp;
#endif
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

Word MainWindow::parseOriginalFileLine(QString &line)
{
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);
    int wordLen = secondSpace - firstSpace - 1;

    if (-1 == firstSpace || -1 == secondSpace)
    {
        qDebug() << "NG @" << line;
        return Word();
    }
#if MAKE_CW_FILE_DEBUG
    qDebug()<<"first space"<<firstSpace<<"2nd:"<<secondSpace;
#endif

    return Word(line.mid(firstSpace + 1, wordLen), line.mid(secondSpace + 1));
}

unsigned int MainWindow::editDist(QString str1, QString str2)
{
//    QStringRef
//    if (min(str1.length(), str2.length()) == 0)
//    {
//        return max(str1.length(), str2.length());
//    }

//    return max(this->editDist(str1.chop(1)))
    return 0;
}