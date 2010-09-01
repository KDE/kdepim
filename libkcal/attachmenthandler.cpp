/*
  This file is part of the kcal library.

  Copyright (c) 2010 Klar�lvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
#include "scheduler.h"

#include <kapplication.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <krun.h>
#include <ktempfile.h>
#include <kio/netaccess.h>

#include <tqfile.h>

namespace KCal {

Attachment *AttachmentHandler::find( TQWidget *parent, const TQString &attachmentName,
                                     Incidence *incidence )
{
  if ( !incidence ) {
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

Attachment *AttachmentHandler::find( TQWidget *parent,
                                     const TQString &attachmentName, const TQString &uid )
{
  if ( uid.isEmpty() ) {
    return 0;
  }

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

  return find( parent, attachmentName, incidence );
}

Attachment *AttachmentHandler::find( TQWidget *parent, const TQString &attachmentName,
                                     ScheduleMessage *message )
{
  if ( !message ) {
    return 0;
  }

  Incidence *incidence = dynamic_cast<Incidence*>( message->event() );
  if ( !incidence ) {
    KMessageBox::error(
      parent,
      i18n( "The calendar invitation stored in this email message is broken in some way. "
            "Unable to continue." ) );
    return 0;
  }

  return find( parent, attachmentName, incidence );
}

static KTempFile *s_tempFile = 0;

static KURL tempFileForAttachment( Attachment *attachment )
{
  KURL url;
  TQStringList patterns = KMimeType::mimeType( attachment->mimeType() )->patterns();
  if ( !patterns.empty() ) {
    s_tempFile = new KTempFile( TQString::null,
                                TQString( patterns.first() ).remove( '*' ), 0600 );
  } else {
    s_tempFile = new KTempFile( TQString::null, TQString::null, 0600 );
  }

  TQFile *qfile = s_tempFile->file();
  qfile->open( IO_WriteOnly );
  TQTextStream stream( qfile );
  stream.writeRawBytes( attachment->decodedData().data(), attachment->size() );
  s_tempFile->close();
  TQFile tf( s_tempFile->name() );
  if ( tf.size() != attachment->size() ) {
    //whoops. failed to write the entire attachment. return an invalid URL.
    delete s_tempFile;
    s_tempFile = 0;
    return url;
  }

  url.setPath( s_tempFile->name() );
  return url;
}

bool AttachmentHandler::view( TQWidget *parent, Attachment *attachment )
{
  if ( !attachment ) {
    return false;
  }

  bool stat = true;
  if ( attachment->isUri() ) {
    kapp->invokeBrowser( attachment->uri() );
  } else {
    // put the attachment in a temporary file and launch it
    KURL tempUrl = tempFileForAttachment( attachment );
    if ( tempUrl.isValid() ) {
      stat = KRun::runURL( tempUrl, attachment->mimeType(), false, true );
    } else {
      stat = false;
      KMessageBox::error(
        parent,
        i18n( "Unable to create a temporary file for the attachment." ) );
    }
    delete s_tempFile;
    s_tempFile = 0;
  }
  return stat;
}

bool AttachmentHandler::view( TQWidget *parent, const TQString &attachmentName, Incidence *incidence )
{
  return view( parent, find( parent, attachmentName, incidence ) );
}

bool AttachmentHandler::view( TQWidget *parent, const TQString &attachmentName, const TQString &uid )
{
  return view( parent, find( parent, attachmentName, uid ) );
}

bool AttachmentHandler::view( TQWidget *parent, const TQString &attachmentName,
                              ScheduleMessage *message )
{
  return view( parent, find( parent, attachmentName, message ) );
}

bool AttachmentHandler::saveAs( TQWidget *parent, Attachment *attachment )
{
  // get the saveas file name
  TQString saveAsFile = KFileDialog::getSaveFileName( attachment->label(), TQString::null, parent,
                                                     i18n( "Save Attachment" ) );
  if ( saveAsFile.isEmpty() ||
       ( TQFile( saveAsFile ).exists() &&
         ( KMessageBox::warningYesNo(
             parent,
             i18n( "%1 already exists. Do you want to overwrite it?").
             arg( saveAsFile ) ) == KMessageBox::No ) ) ) {
    return false;
  }

  bool stat = false;
  if ( attachment->isUri() ) {
    // save the attachment url
    stat = KIO::NetAccess::file_copy( attachment->uri(), KURL( saveAsFile ), -1, true );
  } else {
    // put the attachment in a temporary file and save it
    KURL tempUrl = tempFileForAttachment( attachment );
    if ( tempUrl.isValid() ) {
      stat = KIO::NetAccess::file_copy( tempUrl, KURL( saveAsFile ), -1, true );
      if ( !stat && KIO::NetAccess::lastError() ) {
        KMessageBox::error( parent, KIO::NetAccess::lastErrorString() );
      }
    } else {
      stat = false;
      KMessageBox::error(
        parent,
        i18n( "Unable to create a temporary file for the attachment." ) );
    }
    delete s_tempFile;
    s_tempFile = 0;
  }
  return stat;
}

bool AttachmentHandler::saveAs( TQWidget *parent, const TQString &attachmentName,
                                Incidence *incidence )
{
  return saveAs( parent, find( parent, attachmentName, incidence ) );
}

bool AttachmentHandler::saveAs( TQWidget *parent, const TQString &attachmentName, const TQString &uid )
{
  return saveAs( parent, find( parent, attachmentName, uid ) );
}

bool AttachmentHandler::saveAs( TQWidget *parent, const TQString &attachmentName,
                                ScheduleMessage *message )
{
  return saveAs( parent, find( parent, attachmentName, message ) );
}

}

