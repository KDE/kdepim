/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "activitywarning.h"
#include "activitymanager.h"

#include <KLocalizedString>

namespace PimActivity {
ActivityWarning::ActivityWarning(ActivityManager *activityManager, QWidget *parent)
    : KMessageWidget(parent)
{
    setMessageType( Warning );
    setCloseButtonVisible( true );
    setWordWrap( true );
    setText( i18n( "Activities is not active on this computer." ) );
    connect(activityManager, SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)), this, SLOT(setServiceStatusChanged(KActivities::Consumer::ServiceStatus)));
    setVisible(!activityManager->isActive());
}

ActivityWarning::~ActivityWarning()
{

}

void ActivityWarning::setServiceStatusChanged(KActivities::Consumer::ServiceStatus status)
{
    setVisible(status == KActivities::Consumer::NotRunning);
}

}

