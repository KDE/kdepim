/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "resourcekolabbase.h"
#include "kmailconnection.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kinputdialog.h>
#include <kurl.h>
#include <ktempfile.h>
#include <qtextstream.h>

using namespace Kolab;


ResourceKolabBase::ResourceKolabBase( const QCString& objId )
  : mSilent( false )
{
  mConnection = new KMailConnection( this, objId );
}

ResourceKolabBase::~ResourceKolabBase()
{
  delete mConnection;
}


bool ResourceKolabBase::kmailSubresources( QMap<QString, bool>& lst,
                                           const QString& contentsType ) const
{
  return mConnection->kmailSubresources( lst, contentsType );
}

bool ResourceKolabBase::kmailIncidences( QMap<Q_UINT32, QString>& lst,
                                         const QString& mimetype,
                                         const QString& resource ) const
{
  return mConnection->kmailIncidences( lst, mimetype, resource );
}

bool ResourceKolabBase::kmailGetAttachment( KURL& url, const QString& resource,
                                            Q_UINT32 sernum,
                                            const QString& filename ) const
{
  return mConnection->kmailGetAttachment( url, resource, sernum, filename );
}

bool ResourceKolabBase::kmailDeleteIncidence( const QString& resource,
                                              Q_UINT32 sernum )
{
  return mSilent || mConnection->kmailDeleteIncidence( resource, sernum );
}

bool ResourceKolabBase::kmailUpdate( const QString& resource,
                                     Q_UINT32& sernum,
                                     const QString& xml,
                                     const QCString& subject,
                                     const QStringList& attachments,
                                     const QStringList& deletedAttachments )
{
  if ( mSilent )
    return true;

  // Save the xml file. Will be deleted at the end of this method
  KTempFile file;
  file.setAutoDelete( true );
  QTextStream* stream = file.textStream();
  stream->setEncoding( QTextStream::UnicodeUTF8 );
  *stream << xml;
  file.close();

  // Add the xml file as an attachment
  QStringList a = attachments;
  KURL url;
  url.setPath( file.name() );
  url.setFileEncoding( "UTF-8" );
  a.prepend( url.url() );

  return mConnection->kmailUpdate( resource, sernum, subject, a, deletedAttachments );
}

QString ResourceKolabBase::configFile( const QString& type ) const
{
  return locateLocal( "config",
                      QString( "kresources/kolab/%1rc" ).arg( type ) );
}

bool ResourceKolabBase::connectToKMail() const
{
  return mConnection->connectToKMail();
}

QString ResourceKolabBase::findWritableResource( const ResourceMap& resources )
{
  ResourceMap possible;
  ResourceMap::ConstIterator it;
  for ( it = resources.begin(); it != resources.end(); ++it ) {
    if ( it.data().writable() && it.data().active() )
      // Writable and active possibility
      possible[ it.key() ] = it.data();
  }

  if ( possible.isEmpty() )
    // None found!!
    return QString::null;
  if ( possible.count() == 1 )
    // Just one found
    return possible.begin().key();

  // Several found, ask the user
  // TODO: Show the label instead of the resource name
  return KInputDialog::getItem( i18n( "Select Resource Folder" ),
                                i18n( "You have more than one writable resource folder. "
                                      "Please select the one you want to write to." ),
                                possible.keys() );
}
