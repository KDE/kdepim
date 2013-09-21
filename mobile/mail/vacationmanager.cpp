/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "vacationmanager.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksieveui/util.h>
#include <ksieveui/vacation/vacation.h>

#include <QtCore/QTimer>

VacationManager::VacationManager( KActionCollection *actionCollection, QObject *kernel, QObject *parent )
  : QObject( parent ),
    mKernel( kernel ),
    mVacationScriptIsActive( false ),
    mEditAction( 0 )
{
  if ( KSieveUi::Util::checkOutOfOfficeOnStartup() )
    QTimer::singleShot( 0, this, SLOT(checkVacation()) );

  if ( KSieveUi::Util::allowOutOfOfficeSettings() ) {
    mEditAction = new KAction( i18n( "Edit \"Out of Office\" Replies" ), this );
    actionCollection->addAction( "tools_edit_vacation", mEditAction );
    connect( mEditAction, SIGNAL(triggered(bool)), SLOT(editVacation()) );
  }
}

VacationManager::~VacationManager()
{
}

bool VacationManager::activeVacationScriptAvailable() const
{
  return mVacationScriptIsActive;
}

void VacationManager::updateVacationScriptActivity( bool active, const QString &serverName )
{
  Q_UNUSED(serverName);
  mVacationScriptIsActive = active;
  emit vacationScriptActivityChanged();
}

void VacationManager::checkVacation()
{
  updateVacationScriptActivity( false );

  if ( !askToGoOnline() )
    return;
  
  KSieveUi::Vacation *vacation = new KSieveUi::Vacation( this, true /* check only */ );
  connect( vacation, SIGNAL(scriptActive(bool,QString)), SLOT(updateVacationScriptActivity(bool,QString)) );
  connect( vacation, SIGNAL(requestEditVacation()), SLOT(editVacation()) );
}

void VacationManager::editVacation()
{
  if ( !askToGoOnline() )
    return;

  if ( mVacation )
    return;

  mVacation = new KSieveUi::Vacation( this );
  connect( mVacation, SIGNAL(scriptActive(bool,QString)), SLOT(updateVacationScriptActivity(bool,QString)) );
  connect( mVacation, SIGNAL(requestEditVacation()), SLOT(editVacation()) );
  if ( mVacation->isUsable() ) {
    connect( mVacation, SIGNAL(result(bool)), mVacation, SLOT(deleteLater()) );
  } else {
    QString msg = i18n( "KMail Mobile's Out of Office Reply functionality relies on "
                        "server-side filtering. You have not yet configured an "
                        "IMAP server for this.\n"
                        "You can do this on the \"Filtering\" tab of the IMAP "
                        "account configuration.");
    KMessageBox::sorry( 0, msg, i18n( "No Server-Side Filtering Configured" ) );

    delete mVacation;
  }
}

bool VacationManager::askToGoOnline() const
{
  bool result = false;

  QMetaObject::invokeMethod( mKernel, "askToGoOnline", Qt::DirectConnection,
                             Q_RETURN_ARG( bool, result ) );

  return result;
}

#include "vacationmanager.moc"
