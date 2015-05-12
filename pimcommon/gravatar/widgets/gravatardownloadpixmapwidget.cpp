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
    QLabel *lab = new QLabel(i18n("Email:"));
    lab->setObjectName(QStringLiteral("labemail"));
    hbox->addWidget(lab);
    mLineEdit = new QLineEdit;
    mLineEdit->setObjectName(QStringLiteral("email"));
    connect(mLineEdit, &QLineEdit::textChanged, this, &GravatarDownloadPixmapWidget::slotTextChanged);
    hbox->addWidget(mLineEdit);

    mGetPixmapButton = new QPushButton(i18n("searchbutton"));
    mGetPixmapButton->setObjectName(QStringLiteral("searchbutton"));
    connect(mGetPixmapButton, &QAbstractButton::clicked, this, &GravatarDownloadPixmapWidget::slotSearchButton);
    hbox->addWidget(mGetPixmapButton);
    mGetPixmapButton->setEnabled(false);

    mResultLabel = new QLabel;
    mResultLabel->setObjectName(QStringLiteral("resultlabel"));
    mainLayout->addWidget(mResultLabel);
}

GravatarDownloadPixmapWidget::~GravatarDownloadPixmapWidget()
{

}

void GravatarDownloadPixmapWidget::slotResolvUrlFinish(PimCommon::GravatarResolvUrlJob *job)
{
    if (job) {
        qDebug() << job->hasGravatar();
        if (job->hasGravatar()) {
            mResultLabel->setPixmap(job->pixmap());
        } else {
            //KF5 add i18n
            mResultLabel->setText(i18n("No gravatar found."));
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
        connect(job, &GravatarResolvUrlJob::finished, this, &GravatarDownloadPixmapWidget::slotResolvUrlFinish);
        job->start();
    }
}

void GravatarDownloadPixmapWidget::slotTextChanged(const QString &text)
{
    mGetPixmapButton->setEnabled(!text.trimmed().isEmpty());
}
