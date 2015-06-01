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

#include "gravatarcreatewidget.h"
#include "pimcommon/gravatar/gravatarresolvurljob.h"

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <KLocalizedString>

using namespace KABGravatar;

GravatarCreateWidget::GravatarCreateWidget(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *mainLayout = new QGridLayout;
    setLayout(mainLayout);
    QLabel *lab = new QLabel(i18n("Email:"));
    lab->setObjectName(QStringLiteral("emaillabel"));
    mainLayout->addWidget(lab, 0, 0);

    mEmailLab = new QLabel;
    mEmailLab->setObjectName(QStringLiteral("email"));
    mainLayout->addWidget(mEmailLab, 0, 1);

    mSearchGravatar = new QPushButton(i18n("Search"));
    mSearchGravatar->setEnabled(false);
    mSearchGravatar->setObjectName(QStringLiteral("search"));
    mainLayout->addWidget(mSearchGravatar, 0, 2);
    connect(mSearchGravatar, SIGNAL(clicked(bool)), this, SLOT(slotSearchGravatar()));


    mResultGravatar = new QLabel;
    mResultGravatar->setObjectName(QStringLiteral("result"));
    mainLayout->addWidget(mResultGravatar, 1, 0);
}

GravatarCreateWidget::~GravatarCreateWidget()
{

}

void GravatarCreateWidget::setEmail(const QString &email)
{
    if (mEmail != email) {
        mEmail = email;
        mEmailLab->setText(mEmail);
        mSearchGravatar->setEnabled(!mEmail.trimmed().isEmpty());
    }
}

void GravatarCreateWidget::slotSearchGravatar()
{
    if (!mEmail.isEmpty()) {
        PimCommon::GravatarResolvUrlJob *job = new PimCommon::GravatarResolvUrlJob(this);
        job->setEmail(mEmail);
        job->setUseDefaultPixmap(false);
        connect(job, SIGNAL(finished(PimCommon::GravatarResolvUrlJob*)), this, SLOT(slotSearchGravatarFinished(PimCommon::GravatarResolvUrlJob*)));
        job->start();
    }
}

void GravatarCreateWidget::slotSearchGravatarFinished(PimCommon::GravatarResolvUrlJob *job)
{
    if (job) {
        if (job->hasGravatar()) {
            mResultGravatar->setPixmap(job->pixmap());
        } else {
            mResultGravatar->setText(i18n("No Gravatar Found."));
        }
    }
}
