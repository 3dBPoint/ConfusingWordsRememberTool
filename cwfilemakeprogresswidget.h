#ifndef CWFILEMAKEPROGRESSWIDGET_H
#define CWFILEMAKEPROGRESSWIDGET_H

#include <QWidget>
#include <QTime>
#include <QTimer>

namespace Ui {
class CWFileMakeProgressWidget;
}

class CWFileMakeProgressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CWFileMakeProgressWidget(QWidget *parent = 0);
    ~CWFileMakeProgressWidget();

public slots:
    void onSetProgress(uint32_t progress);
    void onSetFull(uint32_t full);
    void onProgressIncreaseOnece();

private slots:
    void onETARefreshTimeout();

private:
    Ui::CWFileMakeProgressWidget *ui;

    QTime timeElapsed;

    QTimer *etaRefreshTimer = nullptr;

    uint32_t full = 0;
    uint32_t overallProgress = 0;

    int lastTimeElapsed = 0;
    unsigned int dayElapsed = 0;
};

#endif // CWFILEMAKEPROGRESSWIDGET_H
