#include "cwfilemakeprogresswidget.h"
#include "ui_cwfilemakeprogresswidget.h"

#include <QDebug>
#include <stdint.h>

#define MSEC_PER_DAY (1000 * 60 * 60 * 24)

CWFileMakeProgressWidget::CWFileMakeProgressWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CWFileMakeProgressWidget)
{
    ui->setupUi(this);
    timeElapsed.start();

//    etaRefreshTimer = new QTimer(this);
//    connect(etaRefreshTimer, &QTimer::timeout, this, &CWFileMakeProgressWidget::onETARefreshTimeout);
//    etaRefreshTimer->start(1000);
}

CWFileMakeProgressWidget::~CWFileMakeProgressWidget()
{
    qDebug() << "cw file make progress widget deleted";
    delete ui;
}

void CWFileMakeProgressWidget::onSetProgress(uint32_t progress)
{
    ui->ui_progress_bar->setValue(progress);

    ui->ui_lb_progress->setText(QString("%1/%2").arg(progress).arg(full));
}

void CWFileMakeProgressWidget::onSetFull(uint32_t full)
{
    ui->ui_progress_bar->setMaximum(full);
    this->full = full;
}

void CWFileMakeProgressWidget::onProgressIncreaseOnece()
{
    overallProgress += 1;

    ui->ui_progress_bar->setValue(overallProgress);

    ui->ui_lb_progress->setText(QString("%1/%2").arg(overallProgress).arg(full));
}

void CWFileMakeProgressWidget::onETARefreshTimeout()
{
    int thisTimeElapsed = timeElapsed.elapsed();

    int progress = ui->ui_progress_bar->value();
    int full = ui->ui_progress_bar->maximum();

    if (thisTimeElapsed < lastTimeElapsed) // every 24 hours warp to 0
    {
        dayElapsed += 1;
    }

    lastTimeElapsed = thisTimeElapsed;

    double msecPerProg = ((double)thisTimeElapsed + (dayElapsed * MSEC_PER_DAY)) / progress;

    double eta = (full - progress) * msecPerProg;

    int msec, hour, min, sec;

    if (eta > INT_MAX)
    {
        msec = INT_MAX;
        sec = msec / 1000;
        min = sec / 60;
        hour = min / 60;

        min = min - hour * 60;
        sec = sec - hour * 60 * 60 - min * 60;

        ui->ui_lb_progress->setText(QString("%1/%2, ETA > %3hour, %4min, %5sec")
                                    .arg(progress).arg(full).arg(hour).arg(min).arg(sec));
    }
    else
    {
        msec = (int)eta;
        sec = msec / 1000;
        min = sec / 60;
        hour = min / 60;

        min = min - hour * 60;
        sec = sec - hour * 60 * 60 - min * 60;

        ui->ui_lb_progress->setText(QString("%1/%2, ETA:%3hour, %4min, %5sec")
                                    .arg(progress).arg(full).arg(hour).arg(min).arg(sec));

    }
    ui->ui_lb_progress->adjustSize();
}
