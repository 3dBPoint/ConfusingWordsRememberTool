#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cwfilemakeprogresswidget.h"
#include "cwfilemaker.h"
#include "confusingwordspair.h"

#include <QFile>
#include <QMainWindow>
#include <QMessageBox>
#include <QTextStream>
#include <QMap>
#include <QTime>

#define CW_FILE_MAKE_THREAD_NUM 4

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void feedWord(Word w, unsigned int startSearchPos, CWFileMaker *distnation);
    void overallProgress(int prog, int full);
    void showProgWidget();

public slots:
    void onCWFound(int dist, ConfusingWordsPair cwPair);
    void onParsedOnce();
    void onCWFeedWordEaten(CWFileMaker *sponsor);

private slots:
    void on_ui_btn_select_origin_file_clicked();

    // void on_ui_btn_operate_confusing_word_file_clicked();

    void on_ui_btn_next_pair_clicked();

    void on_ui_btn_load_cw_file_clicked();

    void on_ui_btn_make_cw_file_clicked();

private:
    Ui::MainWindow *ui;
    QMessageBox *pMsgBox = nullptr;

    QVector<Word> wordsVec;
    QVector<Word>::iterator wordsVecIt;

    ConfusingWordsPair pCurrentCWPairs;
    QList<ConfusingWordsPair> *pRemainCWPairs;
    QList<ConfusingWordsPair> *pCitedCWPairs;

    QMap<CWFileMaker *, QThread *> cwFileMakerThreads;

    uint32_t cwCalculatedTimes;
    uint32_t cwFullCalculateTimes;
    QTime cwMakeTimer;

    CWFileMakeProgressWidget *progWidget = nullptr;
//    QThread *progWidgetThread = nullptr;

    bool makeCWFiles();

    Word parseOriginalFileLine(QString &line);
    uint32_t getCWPairCalcTimes();

    void showOneCWPairRandomly();
    int32_t getShowPairRandomIndex();
    void showOneCWPair(ConfusingWordsPair pair);

    void RefreshFileList();
    QString cwFileName(int dist);
    bool loadCWFile(QString fileName);

    QString statusBarStudyProgressStr(uint32_t dist, uint32_t index, double prog);
};

#endif // MAINWINDOW_H
