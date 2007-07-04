/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "configguifile.h"

#include <kurlrequester.h>
#include <klocale.h>
#include <kdialog.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qdom.h>

ConfigGuiFile::ConfigGuiFile( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  QBoxLayout *filenameLayout = new QHBoxLayout( topLayout() );

  QLabel *label = new QLabel( i18n("Directory name:"), this );
  filenameLayout->addWidget( label );

  mFilename = new KURLRequester( this );
  mFilename->setMode( KFile::Directory | KFile::LocalOnly );
  filenameLayout->addWidget( mFilename );

  QBoxLayout *recursiveLayout = new QHBoxLayout( topLayout() );

  mRecursive = new QCheckBox( i18n("Sync all subdirectories"), this );
  recursiveLayout->addWidget( mRecursive );

  topLayout()->addStretch( 1 );
}

void ConfigGuiFile::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode n;
  for( n = docElement.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();
    if ( e.tagName() == "path" ) {
      mFilename->setURL( e.text() );
    } else if ( e.tagName() == "recursive" ) {
      mRecursive->setChecked( e.text() == "TRUE" );
    }
  }
}

QString ConfigGuiFile::save() const
{
  QString xml;
  xml = "<config>";
  xml += "<path>" + mFilename->url() + "</path>";
  xml += "<recursive>";
  if ( mRecursive->isChecked() ) xml += "TRUE";
  else xml += "FALSE";
  xml += "</recursive>";
  xml += "</config>";

  return xml;
}
