/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractioncopy.h"

#include <KDE/Akonadi/ItemCopyJob>
#include <KDE/KLocale>
// #include <KDE/KMessageBox>

using namespace MailCommon;

FilterActionCopy::FilterActionCopy( QObject *parent )
  : FilterActionWithFolder( "copy", i18n( "Copy Into Folder" ), parent )
{
}

FilterAction::ReturnCode FilterActionCopy::process( ItemContext &context ) const
{
  // copy the message 1:1
  Akonadi::ItemCopyJob *job = new Akonadi::ItemCopyJob( context.item(), mFolder, 0 );
  connect(job, SIGNAL(result(KJob*)), this, SLOT(jobFinished(KJob*)));

  return GoOn;
}

void FilterActionCopy::jobFinished(KJob* job)
{
  if (job->error()) {
    kError() << "Error while moving mail: " << job->errorString();
//     KMessageBox::error(0, i18n("<qt>Error while copying the mail.<br/>The error was: <b>%1</b>.</qt>").arg(job->errorString()),
//                           i18n("Filter error"));
  }
}


bool FilterActionCopy::requiresBody() const
{
  return false;
}

SearchRule::RequiredPart FilterActionCopy::requiredPart() const
{
    return SearchRule::Envelope;
}

FilterAction* FilterActionCopy::newAction()
{
  return new FilterActionCopy;
}
