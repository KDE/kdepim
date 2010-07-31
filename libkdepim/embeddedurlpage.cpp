/*
    This file is part of libkdepim.

    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
    Part of loadContents() copied from the kpartsdesignerplugin:
    Copyright (C) 2005, David Faure <faure@kde.org>

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

#include "embeddedurlpage.h"
#include <kparts/componentfactory.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>
#include <kmimetype.h>
#include <klocale.h>
#include <tqlayout.h>
#include <tqlabel.h>

using namespace KPIM;

EmbeddedURLPage::EmbeddedURLPage( const TQString &url, const TQString &mimetype,
                                  TQWidget *parent, const char *name )
  : TQWidget( parent, name ), mUri(url), mMimeType( mimetype ), mPart( 0 )
{
  initGUI( url, mimetype );
}

void EmbeddedURLPage::initGUI( const TQString &url, const TQString &/*mimetype*/ )
{
  TQVBoxLayout *layout = new TQVBoxLayout( this );
  layout->setAutoAdd( true );
  new TQLabel( i18n("Showing URL %1").arg( url ), this );
}

void EmbeddedURLPage::loadContents()
{
  if ( !mPart ) {
    if ( mMimeType.isEmpty() || mUri.isEmpty() )
        return;
    TQString mimetype = mMimeType;
    if ( mimetype == "auto" )
        mimetype == KMimeType::findByURL( mUri )->name();
    // "this" is both the parent widget and the parent object
    mPart = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>( mimetype, TQString::null, this, 0, this, 0 );
    if ( mPart ) {
        mPart->openURL( mUri );
        mPart->widget()->show();
    }
//void KParts::BrowserExtension::openURLRequestDelayed( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() )
    KParts::BrowserExtension* be = KParts::BrowserExtension::childObject( mPart );
    connect( be, TQT_SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
//              mPart, TQT_SLOT( openURL( const KURL & ) ) );
             this, TQT_SIGNAL( openURL( const KURL & ) ) );
  }
}

#include "embeddedurlpage.moc"
