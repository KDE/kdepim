/*
    This file is part of KDE Kontact.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "aboutdialog.h"

#include "core.h"
#include "plugin.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <kactivelabel.h>
#include <ktextbrowser.h>

#include <tqlayout.h>
#include <tqlabel.h>

#include <kdebug.h>

using namespace Kontact;

AboutDialog::AboutDialog( Kontact::Core *core, const char *name )
  : KDialogBase( IconList, i18n("About Kontact"), Ok, Ok, core, name, false,
                 true ),
    mCore( core )
{
  addAboutData( i18n( "Kontact Container" ), TQString( "kontact" ),
                KGlobal::instance()->aboutData() );

  TQValueList<Plugin*> plugins = mCore->pluginList();
  TQValueList<Plugin*>::ConstIterator end = plugins.end();
  TQValueList<Plugin*>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it )
    addAboutPlugin( *it );

  addLicenseText( KGlobal::instance()->aboutData() );
}

void AboutDialog::addAboutPlugin( Kontact::Plugin *plugin )
{
  addAboutData( plugin->title(), plugin->icon(), plugin->aboutData() );
}

void AboutDialog::addAboutData( const TQString &title, const TQString &icon,
                                const KAboutData *about )
{
  TQPixmap pixmap = KGlobal::iconLoader()->loadIcon( icon,
                                                    KIcon::Desktop, 48 );

  TQFrame *topFrame = addPage( title, TQString::null, pixmap );

  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  if ( !about ) {
    TQLabel *label = new TQLabel( i18n( "No about information available." ),
                                topFrame );
    topLayout->addWidget( label );
  } else {
    TQString text;

    text += "<p><b>" + about->programName() + "</b><br>";

    text += i18n( "Version %1</p>" ).arg( about->version() );

    if ( !about->shortDescription().isEmpty() ) {
      text += "<p>" + about->shortDescription() + "<br>" +
               about->copyrightStatement() + "</p>";
    }

    TQString home = about->homepage();
    if ( !home.isEmpty() ) {
      text += "<a href=\"" + home + "\">" + home + "</a><br>";
    }

    text.replace( "\n", "<br>" );

    KActiveLabel *label = new KActiveLabel( text, topFrame );
    label->setAlignment( AlignTop );
    topLayout->addWidget( label );


    TQTextEdit *personView = new TQTextEdit( topFrame );
    personView->setReadOnly( true );
    topLayout->addWidget( personView, 1 );

    text = "";

    const TQValueList<KAboutPerson> authors = about->authors();
    if ( !authors.isEmpty() ) {
      text += i18n( "<p><b>Authors:</b></p>" );

      TQValueList<KAboutPerson>::ConstIterator it;
      for ( it = authors.begin(); it != authors.end(); ++it ) {
        text += formatPerson( (*it).name(), (*it).emailAddress() );
        if ( !(*it).task().isEmpty() )
          text += "<i>" + (*it).task() + "</i><br>";
      }
    }

    const TQValueList<KAboutPerson> credits = about->credits();
    if ( !credits.isEmpty() ) {
      text += i18n( "<p><b>Thanks to:</b></p>" );

      TQValueList<KAboutPerson>::ConstIterator it;
      for ( it = credits.begin(); it != credits.end(); ++it ) {
        text += formatPerson( (*it).name(), (*it).emailAddress() );
        if ( !(*it).task().isEmpty() )
          text += "<i>" + (*it).task() + "</i><br>";
      }
    }

    const TQValueList<KAboutTranslator> translators = about->translators();
    if ( !translators.isEmpty() ) {
      text += i18n("<p><b>Translators:</b></p>");

      TQValueList<KAboutTranslator>::ConstIterator it;
      for ( it = translators.begin(); it != translators.end(); ++it ) {
       text += formatPerson( (*it).name(), (*it).emailAddress() );
      }
    }

    personView->setText( text );
  }
}

TQString AboutDialog::formatPerson( const TQString &name, const TQString &email )
{
  TQString text = name;
  if ( !email.isEmpty() ) {
    text += " &lt;<a href=\"mailto:" + email + "\">" + email + "</a>&gt;";
  }

  text += "<br>";
  return text;
}

void AboutDialog::addLicenseText( const KAboutData *about )
{
  if ( !about || about->license().isEmpty() )
    return;

  TQPixmap pixmap = KGlobal::iconLoader()->loadIcon( "signature",
                                                    KIcon::Desktop, 48 );

  TQString title = i18n( "%1 License" ).arg( about->programName() );

  TQFrame *topFrame = addPage( title, TQString::null, pixmap );
  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  KTextBrowser *textBrowser = new KTextBrowser( topFrame );
  textBrowser->setText( TQString( "<pre>%1</pre>" ).arg( about->license() ) );

  topLayout->addWidget( textBrowser );
}

#include "aboutdialog.moc"
