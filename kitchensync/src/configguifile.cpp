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

#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqdom.h>

ConfigGuiFile::ConfigGuiFile( const QSync::Member &member, TQWidget *parent )
  : ConfigGui( member, parent )
{
  TQBoxLayout *filenameLayout = new TQHBoxLayout( topLayout() );

  TQLabel *label = new TQLabel( i18n("Directory name:"), this );
  filenameLayout->addWidget( label );

  mFilename = new KURLRequester( this );
  mFilename->setMode( KFile::Directory | KFile::LocalOnly );
  filenameLayout->addWidget( mFilename );

  TQBoxLayout *recursiveLayout = new TQHBoxLayout( topLayout() );

  mRecursive = new TQCheckBox( i18n("Sync all subdirectories"), this );
  recursiveLayout->addWidget( mRecursive );

  topLayout()->addStretch( 1 );
}

void ConfigGuiFile::load( const TQString &xml )
{
  TQDomDocument doc;
  doc.setContent( xml );
  TQDomElement docElement = doc.documentElement();

  TQDomNode node;
  for ( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    TQDomElement e = node.toElement();
    if ( e.tagName() == "directory" ) {
      TQDomNode subNode;
      for ( subNode = e.firstChild(); !subNode.isNull(); subNode = subNode.nextSibling() ) {
        TQDomElement subElement = subNode.toElement();
        if ( subElement.tagName() == "path" ) {
          mFilename->setURL( subElement.text() );
        } else if ( subElement.tagName() == "recursive" ) {
          mRecursive->setChecked( subElement.text() == "TRUE" );
        }
      }
    }
  }
}

TQString ConfigGuiFile::save() const
{
  TQString xml;
  xml = "<config>\n";
  xml += "  <directory>\n";
  xml += "    <path>" + mFilename->url() + "</path>\n";
  xml += "    <objtype>data</objtype>\n";
  xml += "    <recursive>";
  if ( mRecursive->isChecked() )
    xml += "TRUE";
  else
    xml += "FALSE";
  xml += "</recursive>\n";
  xml += "  </directory>\n";
  xml += "</config>\n";

  return xml;
}
