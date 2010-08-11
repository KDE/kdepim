/*
  This file is part of the kcal library.

  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and provides
  static functions for dealing with calendar incidence attachments.

  @brief
  vCalendar/iCalendar attachment handling.

  @author Allen Winter \<winter@kde.org\>
*/
#include "attachmenthandler.h"
#include "attachment.h"
#include "calendarresources.h"
#include "incidence.h"

#include <kapplication.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <krun.h>
#include <ktempfile.h>
#include <kio/netaccess.h>

#include <qfile.h>

namespace KCal {

Attachment *AttachmentHandler::find( QWidget *parent,
                                     const QString &attachmentName, const QString &uid )
{
  CalendarResources *cal = new CalendarResources( "UTC" );
  cal->readConfig();
  cal->load();
  Incidence *incidence = cal->incidence( uid );
  if ( !incidence ) {
    KMessageBox::error(
      parent,
      i18n( "The incidence that owns the attachment named \"%1\" could not be found. "
            "Perhaps it was removed from your calendar?" ).arg( attachmentName ) );
    return 0;
  }

  // get the attachment by name from the incidence
  Attachment::List as = incidence->attachments();
  Attachment *a = 0;
  if ( as.count() > 0 ) {
    Attachment::List::ConstIterator it;
    for ( it = as.begin(); it != as.end(); ++it ) {
      if ( (*it)->label() == attachmentName ) {
        a = *it;
        break;
      }
    }
  }

  if ( !a ) {
    KMessageBox::error(
      parent,
      i18n( "No attachment named \"%1\" found in the incidence." ).arg( attachmentName ) );
    return 0;
  }

  if ( a->isUri() ) {
    if ( !KIO::NetAccess::exists( a->uri(), true, parent ) ) {
      KMessageBox::sorry(
        parent,
        i18n( "The attachment \"%1\" is a web link that is inaccessible from this computer. " ).
        arg( KURL::decode_string( a->uri() ) ) );
      return 0;
    }
  }
  return a;
}

bool AttachmentHandler::view( QWidget *parent, const QString &attachmentName, const QString &uid )
{
  Attachment *a = find( parent, attachmentName, uid );
  if ( !a ) {
    return false;
  }

  bool stat = true;
  if ( a->isUri() ) {
    kapp->invokeBrowser( a->uri() );
  } else {
    // put the attachment in a temporary file and launch it
    KTempFile *file;
    QStringList patterns = KMimeType::mimeType( a->mimeType() )->patterns();
    if ( !patterns.empty() ) {
      file = new KTempFile( QString::null,
                            QString( patterns.first() ).remove( '*' ), 0600 );
    } else {
      file = new KTempFile( QString::null, QString::null, 0600 );
    }
    file->file()->open( IO_WriteOnly );
    QTextStream stream( file->file() );
    stream.writeRawBytes( a->decodedData().data(), a->size() );
    file->close();

    stat = KRun::runURL( KURL( file->name() ), a->mimeType(), 0, true );
    delete file;
  }
  return stat;
}

bool AttachmentHandler::saveAs( QWidget *parent, const QString &attachmentName, const QString &uid )
{
  Attachment *a = find( parent, attachmentName, uid );
  if ( !a ) {
    return false;
  }

  // get the saveas file name
  QString saveAsFile =
    KFileDialog::getSaveFileName( attachmentName, QString::null, parent, i18n( "Save Attachment" ) );
  if ( saveAsFile.isEmpty() ||
       ( QFile( saveAsFile ).exists() &&
         ( KMessageBox::warningYesNo(
             parent,
             i18n( "%1 already exists. Do you want to overwrite it?").
             arg( saveAsFile ) ) == KMessageBox::No ) ) ) {
    return false;
  }

  bool stat = false;
  if ( a->isUri() ) {
    // save the attachment url
    stat = KIO::NetAccess::file_copy( a->uri(), KURL( saveAsFile ), -1, true );
  } else {
    // put the attachment in a temporary file and save it
    KTempFile *file;
    QStringList patterns = KMimeType::mimeType( a->mimeType() )->patterns();
    if ( !patterns.empty() ) {
      file = new KTempFile( QString::null,
                            QString( patterns.first() ).remove( '*' ), 0600 );
    } else {
      file = new KTempFile( QString::null, QString::null, 0600 );
    }
    file->file()->open( IO_WriteOnly );
    QTextStream stream( file->file() );
    stream.writeRawBytes( a->decodedData().data(), a->size() );
    file->close();

    stat = KIO::NetAccess::file_copy( KURL( file->name() ), KURL( saveAsFile ), -1, true );
    delete file;
  }
  return stat;
}

}

