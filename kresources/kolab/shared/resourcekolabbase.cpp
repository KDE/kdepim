/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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
#include <kmessagebox.h>
#include <qtextstream.h>
#include <kdebug.h>

using namespace Kolab;

static unsigned int uniquifier = 0;

ResourceKolabBase::ResourceKolabBase( const QCString& objId )
  : mSilent( false )
{
  KGlobal::locale()->insertCatalogue( "kres_kolab" );
  QString uniqueObjId = QString( objId ) + QString::number( uniquifier++ );
  mConnection = new KMailConnection( this, uniqueObjId.utf8() );
}

ResourceKolabBase::~ResourceKolabBase()
{
  delete mConnection;
}


bool ResourceKolabBase::kmailSubresources( QValueList<KMailICalIface::SubResource>& lst,
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
                                     const QString& mimetype,
                                     const QString& subject,
                                     const QStringList& _attachmentURLs,
                                     const QStringList& _attachmentMimetypes,
                                     const QStringList& _attachmentNames,
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
  QStringList attachmentURLs = _attachmentURLs;
  QStringList attachmentMimeTypes = _attachmentMimetypes;
  QStringList attachmentNames = _attachmentNames;
  KURL url;
  url.setPath( file.name() );
  url.setFileEncoding( "UTF-8" );
  attachmentURLs.prepend( url.url() );
  attachmentMimeTypes.prepend( mimetype );
  attachmentNames.prepend( "kolab.xml" );

  QString subj = subject;
  if ( subj.isEmpty() )
    subj = i18n("Internal kolab data: Do not delete this mail.");

  return mConnection->kmailUpdate( resource, sernum, subj,
                                   attachmentURLs, attachmentMimeTypes, attachmentNames,
                                   deletedAttachments );
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
  // I have to use the label (shown in the dialog) as key here. But given how the
  // label is made up, it should be unique. If it's not, well the dialog would suck anyway...
  QMap<QString, QString> possible;
  QStringList labels;
  ResourceMap::ConstIterator it;
  for ( it = resources.begin(); it != resources.end(); ++it ) {
    if ( it.data().writable() && it.data().active() ) {
      // Writable and active possibility
      possible[ it.data().label() ] = it.key();
    }
  }

  if ( possible.isEmpty() ) { // None found!!
    kdWarning(5650) << "No writable resource found!" << endl;
    KMessageBox::error( 0, i18n( "No writable resource was found, saving will not be possible. Reconfigure KMail first." ) );
    return QString::null;
  }
  if ( possible.count() == 1 )
    // Just one found
    return possible.begin().data(); // yes this is the subresource key, i.e. location

  // Several found, ask the user
  QString chosenLabel = KInputDialog::getItem( i18n( "Select Resource Folder" ),
                                               i18n( "You have more than one writable resource folder. "
                                                     "Please select the one you want to write to." ),
                                               possible.keys() );
  if ( chosenLabel.isEmpty() ) // cancelled
    return QString::null;
  return possible[chosenLabel];
}

