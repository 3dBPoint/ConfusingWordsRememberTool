#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "confusingwordspair.h"

#include <QFile>
#include <QMainWindow>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_ui_btn_select_origin_file_clicked();

    void on_ui_cmb_box_edit_distance_currentTextChanged(const QString &arg1);

    void on_ui_btn_operate_confusing_word_file_clicked();

private:
    Ui::MainWindow *ui;
    QMessageBox *pMsgBox = nullptr;

    QVector<Word> wordsVec;
    QList<ConfusingWordsPair> *pRemainCWPairs;
    QList<ConfusingWordsPair> *pShownCWPairs;

    QString originalFileName;
    QString confusingFileName;

    bool makeCWFile(QString cwFileName, unsigned int dist);
    bool makeCWFile(QString originalName, QString cwName, unsigned int dist);

    Word parseOriginalFileLine(QString &line);

    uint32_t getShowPairRandomIndex();
    void showOneCWPair(ConfusingWordsPair pair);
};

#endif // MAINWINDOW_H
