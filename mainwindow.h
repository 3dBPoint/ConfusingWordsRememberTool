#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QMainWindow>



struct Word_explaination
{
    QString partOfSpeech;
    QString chinese;
};

class Word
{
public:
    Word(){}

    Word(QString wo, QString ex):
        word(wo),
        exp(ex)
    {}

    QString word;
    QString exp;
//    QVector<Word_explaination> expVec;
};

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

    QString originalFileName;
    QString originalFileStr;
    QVector<Word> wordsVec;

    QString confusingFileName;

    bool makeCWFile(QString originalName, QString cwName, unsigned int dist);
    Word parseOriginalFileLine(QString &line);
    unsigned int editDist(QString str1, QString str2);
};

#endif // MAINWINDOW_H
