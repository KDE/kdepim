/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "gravatardownloadpixmapwidget.h"
#include "gravatar/gravatarresolvurljob.h"
#include <QLabel>
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDebug>

using namespace PimCommon;
GravatarDownloadPixmapWidget::GravatarDownloadPixmapWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QHBoxLayout *hbox = new QHBoxLayout;
    mainLayout->addLayout(hbox);
    QLabel *lab = new QLabel(QLatin1String("Email:"));
    lab->setObjectName(QLatin1String("labemail"));
    hbox->addWidget(lab);
    mLineEdit = new QLineEdit;
    mLineEdit->setObjectName(QLatin1String("email"));
    connect(mLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
    hbox->addWidget(mLineEdit);

    mGetPixmapButton = new QPushButton(QLatin1String("searchbutton"));
    mGetPixmapButton->setObjectName(QLatin1String("searchbutton"));
    connect(mGetPixmapButton, SIGNAL(clicked(bool)), this, SLOT(slotSearchButton()));
    hbox->addWidget(mGetPixmapButton);
    mGetPixmapButton->setEnabled(false);

    mResultLabel = new QLabel;
    mResultLabel->setObjectName(QLatin1String("resultlabel"));
    mainLayout->addWidget(mResultLabel);
}

GravatarDownloadPixmapWidget::~GravatarDownloadPixmapWidget()
{

}

QPixmap GravatarDownloadPixmapWidget::gravatarPixmap() const
{
    return mGravatarPixmap;
}

void GravatarDownloadPixmapWidget::slotResolvUrlFinish(PimCommon::GravatarResolvUrlJob *job)
{
    if (job) {
        qDebug() << job->hasGravatar();
        if (job->hasGravatar()) {
            mGravatarPixmap = job->pixmap();
            mResultLabel->setPixmap(mGravatarPixmap);
        } else {
            //KF5 add i18n
            mGravatarPixmap = QPixmap();
            mResultLabel->setText(QLatin1String("No gravatar found."));
        }
    }
}

void GravatarDownloadPixmapWidget::slotSearchButton()
{
    PimCommon::GravatarResolvUrlJob *job = new PimCommon::GravatarResolvUrlJob(this);
    job->setEmail(mLineEdit->text());
    //For testing
    //job->setUseDefaultPixmap(true);
    if (job->canStart()) {
        connect(job, SIGNAL(finished(PimCommon::GravatarResolvUrlJob*)), this, SLOT(slotResolvUrlFinish(PimCommon::GravatarResolvUrlJob*)));
        job->start();
    }
}

void GravatarDownloadPixmapWidget::slotTextChanged(const QString &text)
{
    mGetPixmapButton->setEnabled(!text.trimmed().isEmpty());
}
