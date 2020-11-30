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
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>

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
    // void overallProgress(int prog, int full);
    void showProgWidget();

public slots:
    void onCWFound(int dist, ConfusingWordsPair cwPair);
    void onCWFeedWordEaten(CWFileMaker *sponsor);

private slots:
    void on_ui_btn_select_origin_file_clicked();

    // void on_ui_btn_operate_confusing_word_file_clicked();

    void on_ui_btn_next_pair_clicked();

    void on_ui_btn_load_cw_file_clicked();

    void on_ui_btn_make_cw_file_clicked();

    void on_ui_btn_remove_the_pair_clicked();

    void on_ui_btn_remembered_the_pair_once_clicked();

    void on_ui_btn_forget_the_pair_clicked();

    void on_ui_btn_show_meaning_clicked(bool checked);

    void onCWSaveTimerTimeout();

protected:
    void keyReleaseEvent(QKeyEvent *e);

private:
    Ui::MainWindow *ui;
    QMessageBox *pMsgBox = nullptr;

    QVector<Word> wordsVec;
    QVector<Word>::iterator wordsVecIt;

    QList<ConfusingWordsPair> *pRemainCWPairs = nullptr; // Have not remembered, not shown
    QList<ConfusingWordsPair> *pShownCWPairs = nullptr; // words remembered in this period, will display in next period
    QList<ConfusingWordsPair> *pCitedCWPairs = nullptr; // words recited

    QMap<CWFileMaker *, QThread *> cwFileMakerThreads;

    uint32_t cwFullCalculateTimes = 0;
    QTime cwMakeTimer;

    QTimer *cwSaveTimer = nullptr;

    uint32_t selctedDist = 0;

    CWFileMakeProgressWidget *progWidget = nullptr;
//    QThread *progWidgetThread = nullptr;
    int32_t currentShowWordIndex = -1;

    bool makeCWFiles();

    Word parseOriginalFileLine(QString &line);
    uint32_t getCWPairCalcTimes();

    int32_t getShowPairRandomIndex();
    void showOneCWPairRandomly();
    void showOneCWPair(ConfusingWordsPair pair);

    void RefreshFileList();
    QString cwFileName(int dist);
    bool loadCWFile(QString fileName);

    QString statusBarStudyProgressStr(int remain, int shown, int recited, double prog);

    void displayCurrentPairExp(bool left, bool right);
};

#endif // MAINWINDOW_H
