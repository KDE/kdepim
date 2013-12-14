/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "vacationpagewidget.h"
#include "vacationeditwidget.h"
#include "vacationwarningwidget.h"

#include <kmanagesieve/sievejob.h>

#include <KUrl>
#include <KDebug>

#include <QVBoxLayout>

using namespace KSieveUi;
VacationPageWidget::VacationPageWidget(QWidget *parent)
    : QWidget(parent),
      mSieveJob(0)
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);

    mVacationWarningWidget = new VacationWarningWidget;
    lay->addWidget(mVacationWarningWidget);

    mVacationEditWidget = new VacationEditWidget;
    lay->addWidget(mVacationEditWidget);
    setLayout(lay);
}

VacationPageWidget::~VacationPageWidget()
{
    if ( mSieveJob )
        mSieveJob->kill();
    mSieveJob = 0;
}

void VacationPageWidget::setServerUrl(const KUrl &url)
{
    mSieveJob = KManageSieve::SieveJob::get( url );
    connect( mSieveJob, SIGNAL(gotScript(KManageSieve::SieveJob*,bool,QString,bool)),
             SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

void VacationPageWidget::slotGetResult( KManageSieve::SieveJob * job, bool success, const QString & script, bool active )
{
    kDebug() << success
             << ", ?," << active << ")" << endl
             << "script:" << endl
             << script;
    mSieveJob = 0; // job deletes itself after returning from this slot!
    //TODO
}
