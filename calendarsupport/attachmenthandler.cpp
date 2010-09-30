/*
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

#include <KFileDialog>
#include <KLocale>
#include <KMessageBox>
#include <KMimeType>
#include <KRun>
#include <KTemporaryFile>
#include <KToolInvocation>
#include <KIO/NetAccess>

#include <QFile>

using namespace KCalCore;

namespace CalendarSupport {

Attachment::Ptr AttachmentHandler::find( const QString &attachmentName,
                                         const Incidence::Ptr &incidence,
                                         QWidget *parent )
{
  if ( !incidence ) {
    return Attachment::Ptr();
  }

  // get the attachment by name from the incidence
  Attachment::List as = incidence->attachments();
  Attachment::Ptr a;
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
      i18n( "No attachment named \"%1\" found in the incidence.", attachmentName ) );
    return Attachment::Ptr();
  }

  if ( a->isUri() ) {
    if ( !KIO::NetAccess::exists( a->uri(), KIO::NetAccess::SourceSide, parent ) ) {
      KMessageBox::sorry(
        parent,
        i18n( "The attachment \"%1\" is a web link that is inaccessible from this computer. ",
              KUrl::fromPercentEncoding( a->uri().toLatin1() ) ) );
      return Attachment::Ptr();
    }
  }
  return a;
}

Attachment::Ptr AttachmentHandler::find( const QString &attachmentName,
                                         const QString &uid, QWidget *parent )
{
  if ( uid.isEmpty() ) {
    return Attachment::Ptr();
  }

  CalendarResources *cal = new CalendarResources( "UTC" );
  cal->readConfig();
  cal->load();
  Incidence::Ptr incidence = cal->incidence( uid );
  if ( !incidence ) {
    KMessageBox::error(
      parent,
      i18n( "The incidence that owns the attachment named \"%1\" could not be found. "
            "Perhaps it was removed from your calendar?", attachmentName ) );
    return Attachment::Ptr();
  }

  return find( parent, attachmentName, incidence );
}

Attachment::Ptr AttachmentHandler::find( const QString &attachmentName,
                                         const ScheduleMessage::Ptr &message,
                                         QWidget *parent )
{
  if ( !message ) {
    return Attachment::Ptr();
  }

  Incidence::Ptr incidence = message->event().dynamicCast<Incidence>();
  if ( !incidence ) {
    KMessageBox::error(
      parent,
      i18n( "The calendar invitation stored in this email message is broken in some way. "
            "Unable to continue." ) );
    return Attachment::Ptr();
  }

  return find( parent, attachmentName, incidence );
}

static KTemporaryFile *s_tempFile = 0;

static KUrl tempFileForAttachment( const Attachment::Ptr &attachment )
{
  KUrl url;

  s_tempFile = new KTemporaryFile();
  s_tempFile->setAutoRemove( false );
  QStringList patterns = KMimeType::mimeType( attachment->mimeType() )->patterns();
  if ( !patterns.empty() ) {
    s_tempFile->setSuffix( QString( patterns.first() ).remove( '*' ) );
  }
  s_tempFile->open();
  s_tempFile->setPermissions( QFile::ReadUser );
  s_tempFile->write( QByteArray::fromBase64( attachment->data() ) );
  s_tempFile->close();
  QFile tf( s_tempFile->fileName() );
  if ( tf.size() != attachment->size() ) {
    //whoops. failed to write the entire attachment. return an invalid URL.
    delete s_tempFile;
    s_tempFile = 0;
    return url;
  }

  url.setPath( s_tempFile->fileName() );
  return url;
}

bool AttachmentHandler::view( const Attachment::Ptr &attachment, QWidget *parent )
{
  if ( !attachment ) {
    return false;
  }

  bool stat = true;
  if ( attachment->isUri() ) {
    KToolInvocation::invokeBrowser( attachment->uri() );
  } else {
    // put the attachment in a temporary file and launch it
    KUrl tempUrl = tempFileForAttachment( attachment );
    if ( tempUrl.isValid() ) {
      stat = KRun::runUrl( tempUrl, attachment->mimeType(), 0, true );
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

bool AttachmentHandler::view( const QString &attachmentName,
                              const Incidence::Ptr &incidence, QWidget *parent )
{
  return view( parent, find( parent, attachmentName, incidence ) );
}

bool AttachmentHandler::view( const QString &attachmentName, const QString &uid, QWidget *parent )
{
  return view( parent, find( parent, attachmentName, uid ) );
}

bool AttachmentHandler::view( const QString &attachmentName,
                              const ScheduleMessage::Ptr &message, QWidget *parent )
{
  return view( parent, find( parent, attachmentName, message ) );
}

bool AttachmentHandler::saveAs( const Attachment::Ptr &attachment, QWidget *parent )
{
  // get the saveas file name
  QString saveAsFile = KFileDialog::getSaveFileName( attachment->label(), QString(), parent,
                                                     i18n( "Save Attachment" ) );
  if ( saveAsFile.isEmpty() ||
       ( QFile( saveAsFile ).exists() &&
         ( KMessageBox::warningYesNo(
             parent,
             i18n( "%1 already exists. Do you want to overwrite it?",
                   saveAsFile ) ) == KMessageBox::No ) ) ) {
    return false;
  }

  bool stat = false;
  if ( attachment->isUri() ) {
    // save the attachment url
    stat = KIO::NetAccess::file_copy( attachment->uri(), KUrl( saveAsFile ) );
  } else {
    // put the attachment in a temporary file and save it
    KUrl tempUrl = tempFileForAttachment( attachment );
    if ( tempUrl.isValid() ) {
      stat = KIO::NetAccess::file_copy( tempUrl, KUrl( saveAsFile ) );
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

bool AttachmentHandler::saveAs( const QString &attachmentName,
                                const Incidence::Ptr &incidence, QWidget *parent )
{
  return saveAs( parent, find( parent, attachmentName, incidence ) );
}

bool AttachmentHandler::saveAs( const QString &attachmentName, const QString &uid, QWidget *parent )
{
  return saveAs( parent, find( parent, attachmentName, uid ) );
}

bool AttachmentHandler::saveAs( const QString &attachmentName,
                                const ScheduleMessage::Ptr &message, QWidget *parent )
{
  return saveAs( parent, find( parent, attachmentName, message ) );
}

} // namespace CalendarSupport

