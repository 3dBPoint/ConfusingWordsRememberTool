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

    // void on_ui_btn_operate_confusing_word_file_clicked();

    void on_ui_btn_next_pair_clicked();

    void on_ui_btn_load_cw_file_clicked();

    void on_ui_btn_make_cw_file_clicked();

private:
    Ui::MainWindow *ui;
    QMessageBox *pMsgBox = nullptr;

    QVector<Word> wordsVec;

    ConfusingWordsPair pCurrentCWPairs;
    QList<ConfusingWordsPair> *pRemainCWPairs;
    QList<ConfusingWordsPair> *pCitedCWPairs;

    QVector<QList<ConfusingWordsPair> *> CWWordsPairs;

    bool makeCWFiles();
#if 0
    bool makeCWFile(QString originalName, QString cwName, unsigned int dist);
#endif

    Word parseOriginalFileLine(QString &line);

    void showOneCWPairRandomly();
    int32_t getShowPairRandomIndex();
    void showOneCWPair(ConfusingWordsPair pair);

    void RefreshFileList();
    QString cwFileName(int dist);
    bool loadCWFile(QString fileName);
};

#endif // MAINWINDOW_H
