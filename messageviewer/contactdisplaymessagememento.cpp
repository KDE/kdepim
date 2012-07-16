/* Copyright (C) 2012 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "contactdisplaymessagememento.h"

using namespace MessageViewer;

ContactDisplayMessageMemento::ContactDisplayMessageMemento( const QString &emailAddress )
  : ContactAbstractMemento( emailAddress ),
    mMailAllowToRemoteContent( false ),
    mForceDisplayTo( Unknown )
{
}

ContactDisplayMessageMemento::~ContactDisplayMessageMemento()
{
}

bool ContactDisplayMessageMemento::allowToRemoteContent() const
{
  return mMailAllowToRemoteContent;
}

bool ContactDisplayMessageMemento::forceToHtml() const
{
  return ( mForceDisplayTo == Html );
}

bool ContactDisplayMessageMemento::forceToText() const
{
  return ( mForceDisplayTo == Text );
}

void ContactDisplayMessageMemento::processAddress( const KABC::Addressee& addressee )
{
  const QStringList customs = addressee.customs();
  Q_FOREACH( const QString& custom, customs )
  {
    if ( custom == QLatin1String( "MailPreferedFormatting") ) {
      const QString value = addressee.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "MailPreferedFormatting" ) );
      if ( value == QLatin1String( "TEXT" ) ) {
        mForceDisplayTo = Text;
      } else if ( value == QLatin1String( "HTML" ) ) {
        mForceDisplayTo = Html;
      } else {
        mForceDisplayTo = Unknown;
      }
    } else if ( custom == QLatin1String( "MailAllowToRemoteContent") ) {
      const QString value = addressee.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "MailAllowToRemoteContent" ) );
      mMailAllowToRemoteContent = ( value == QLatin1String( "TRUE" ) );
    }
  }
}

#include "contactdisplaymessagememento.moc"
