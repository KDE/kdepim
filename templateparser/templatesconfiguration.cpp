/*   -*- mode: C++; c-file-style: "gnu" -*-
 *   kmail: KDE mail client
 *   Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "templatesconfiguration.h"
#include "ui_templatesconfiguration_base.h"
#include "templatesconfiguration_kfg.h"
#include "globalsettings_base.h"

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

#include <QWhatsThis>
#include <qfont.h>

TemplatesConfiguration::TemplatesConfiguration( QWidget *parent, const char *name )
  : QWidget( parent )
{
  setupUi(this);
  setObjectName(name);

  const QFont f = KGlobalSettings::fixedFont();
  textEdit_new->setFont( f );
  textEdit_reply->setFont( f );
  textEdit_reply_all->setFont( f );
  textEdit_forward->setFont( f );

  setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  sizeHint();

  connect( textEdit_new, SIGNAL(textChanged()),
           this, SLOT(slotTextChanged()) );
  connect( textEdit_reply, SIGNAL(textChanged()),
           this, SLOT(slotTextChanged()) );
  connect( textEdit_reply_all, SIGNAL(textChanged()),
           this, SLOT(slotTextChanged()) );
  connect( textEdit_forward, SIGNAL(textChanged()),
           this, SLOT(slotTextChanged()) );
  connect( lineEdit_quote, SIGNAL(textChanged(QString)),
           this, SLOT(slotTextChanged()) );

  connect( mInsertCommand, SIGNAL(insertCommand(QString,int)),
           this, SLOT(slotInsertCommand(QString,int)) );

  mHelpString =
    i18n( "<p>Here you can create and manage templates to use when "
	  "composing new messages, replies or forwarded messages.</p>"
	  "<p>The message templates support substitution commands, "
	  "either simply type them or select them from "
	  "the <i>Insert command</i> menu.</p>" );
  if ( QString( name ) == "folder-templates" ) {
    mHelpString +=
      i18n( "<p>Templates specified here are folder-specific. "
            "They override both global templates and per-identity "
            "templates.</p>" );
  } else if ( QString( name ) == "identity-templates" ) {
    mHelpString +=
      i18n( "<p>Templates specified here are identity-specific. "
            "They override global templates, but can be overridden by "
	    "per-folder templates if they are specified.</p>" );
  } else {
    mHelpString +=
      i18n( "<p>These are global (default) templates. They can be overridden "
            "by per-identity templates or per-folder templates "
            "if they are specified.</p>" );
  }

  mHelp->setText( i18n( "<a href=\"whatsthis\">How does this work?</a>" ) );
  connect( mHelp, SIGNAL(linkActivated(QString)),
           this, SLOT(slotHelpLinkClicked(QString)) );
}


void TemplatesConfiguration::slotHelpLinkClicked( const QString& )
{
  QWhatsThis::showText( QCursor::pos(), mHelpString );
}

void TemplatesConfiguration::slotTextChanged()
{
  emit changed();
}

void TemplatesConfiguration::resetToDefault()
{
  textEdit_new->setText( DefaultTemplates::defaultNewMessage() );
  textEdit_reply->setText( DefaultTemplates::defaultReply() );
  textEdit_reply_all->setText( DefaultTemplates::defaultReplyAll() );
  textEdit_forward->setText( DefaultTemplates::defaultForward() );
  lineEdit_quote->setText( DefaultTemplates::defaultQuoteString() );
}

void TemplatesConfiguration::loadFromGlobal()
{
  QString str;
  str = TemplateParser::GlobalSettings::self()->templateNewMessage();
  if ( str.isEmpty() ) {
    textEdit_new->setText( DefaultTemplates::defaultNewMessage() );
  } else {
    textEdit_new->setText(str);
  }
  str = TemplateParser::GlobalSettings::self()->templateReply();
  if ( str.isEmpty() ) {
    textEdit_reply->setText( DefaultTemplates::defaultReply() );
  } else {
    textEdit_reply->setText( str );
  }
  str = TemplateParser::GlobalSettings::self()->templateReplyAll();
  if ( str.isEmpty() ) {
    textEdit_reply_all->setText( DefaultTemplates::defaultReplyAll() );
  } else {
    textEdit_reply_all->setText( str );
  }
  str = TemplateParser::GlobalSettings::self()->templateForward();
  if ( str.isEmpty() ) {
    textEdit_forward->setText( DefaultTemplates::defaultForward() );
  } else {
    textEdit_forward->setText( str );
  }
  str = TemplateParser::GlobalSettings::self()->quoteString();
  if ( str.isEmpty() ) {
    lineEdit_quote->setText( DefaultTemplates::defaultQuoteString() );
  } else {
    lineEdit_quote->setText( str );
  }
}

void TemplatesConfiguration::saveToGlobal()
{
  TemplateParser::GlobalSettings::self()->setTemplateNewMessage( strOrBlank( textEdit_new->toPlainText() ) );
  TemplateParser::GlobalSettings::self()->setTemplateReply( strOrBlank( textEdit_reply->toPlainText() ) );
  TemplateParser::GlobalSettings::self()->setTemplateReplyAll( strOrBlank( textEdit_reply_all->toPlainText() ) );
  TemplateParser::GlobalSettings::self()->setTemplateForward( strOrBlank( textEdit_forward->toPlainText() ) );
  TemplateParser::GlobalSettings::self()->setQuoteString( lineEdit_quote->text() );
  TemplateParser::GlobalSettings::self()->writeConfig();
}

void TemplatesConfiguration::loadFromIdentity( uint id )
{
  Templates t( configIdString( id ) );

  QString str;

  str = t.templateNewMessage();
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->templateNewMessage();
  }
  if ( str.isEmpty() ) {
    str = DefaultTemplates::defaultNewMessage();
  }
  textEdit_new->setText( str );

  str = t.templateReply();
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->templateReply();
  }
  if ( str.isEmpty() ) {
    str = DefaultTemplates::defaultReply();
  }
  textEdit_reply->setText( str );

  str = t.templateReplyAll();
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->templateReplyAll();
  }
  if ( str.isEmpty() ) {
    str = DefaultTemplates::defaultReplyAll();
  }
  textEdit_reply_all->setText( str );

  str = t.templateForward();
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->templateForward();
  }
  if ( str.isEmpty() ) {
    str = DefaultTemplates::defaultForward();
  }
  textEdit_forward->setText( str );

  str = t.quoteString();
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->quoteString();
  }
  if ( str.isEmpty() ) {
    str = DefaultTemplates::defaultQuoteString();
  }
  lineEdit_quote->setText( str );
}

void TemplatesConfiguration::saveToIdentity( uint id )
{
  Templates t( configIdString( id ) );
  t.setTemplateNewMessage( strOrBlank( textEdit_new->toPlainText() ) );
  t.setTemplateReply( strOrBlank( textEdit_reply->toPlainText() ) );
  t.setTemplateReplyAll( strOrBlank( textEdit_reply_all->toPlainText() ) );
  t.setTemplateForward( strOrBlank( textEdit_forward->toPlainText() ) );
  t.setQuoteString( lineEdit_quote->text() );
  t.writeConfig();
}

void TemplatesConfiguration::loadFromFolder( const QString &id, uint identity )
{
  Templates t( id );
  Templates* tid = 0;

  if ( identity ) {
    tid = new Templates( configIdString( identity ) );
  }

  QString str;

  str = t.templateNewMessage();
  if ( str.isEmpty() && tid ) {
    str = tid->templateNewMessage();
  }
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->templateNewMessage();
  }
  if ( str.isEmpty() ) {
    str = DefaultTemplates::defaultNewMessage();
  }
  textEdit_new->setText( str );

  str = t.templateReply();
  if ( str.isEmpty() && tid ) {
    str = tid->templateReply();
  }
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->templateReply();
  }
  if ( str.isEmpty() ) {
    str = DefaultTemplates::defaultReply();
  }
  textEdit_reply->setText( str );

  str = t.templateReplyAll();
  if ( str.isEmpty() && tid ) {
    str = tid->templateReplyAll();
  }
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->templateReplyAll();
  }
  if ( str.isEmpty() ) {
    str = DefaultTemplates::defaultReplyAll();
  }
  textEdit_reply_all->setText( str );

  str = t.templateForward();
  if ( str.isEmpty() && tid ) {
    str = tid->templateForward();
  }
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->templateForward();
  }
  if ( str.isEmpty() ) {
    str = DefaultTemplates::defaultForward();
  }
  textEdit_forward->setText( str );

  str = t.quoteString();
  if ( str.isEmpty() && tid ) {
    str = tid->quoteString();
  }
  if ( str.isEmpty() ) {
    str = TemplateParser::GlobalSettings::self()->quoteString();
  }
  if ( str.isEmpty() ) {
      str = DefaultTemplates::defaultQuoteString();
  }
  lineEdit_quote->setText( str );

  delete(tid);
}

void TemplatesConfiguration::saveToFolder( const QString &id )
{
  Templates t( id );

  t.setTemplateNewMessage( strOrBlank( textEdit_new->toPlainText() ) );
  t.setTemplateReply( strOrBlank( textEdit_reply->toPlainText() ) );
  t.setTemplateReplyAll( strOrBlank( textEdit_reply_all->toPlainText() ) );
  t.setTemplateForward( strOrBlank( textEdit_forward->toPlainText() ) );
  t.setQuoteString( lineEdit_quote->text() );
  t.writeConfig();
}

void TemplatesConfiguration::slotInsertCommand( const QString &cmd, int adjustCursor )
{
  KTextEdit* edit;

  if( toolBox1->widget( toolBox1->currentIndex() ) == page_new ) {
    edit = textEdit_new;
  } else if( toolBox1->widget( toolBox1->currentIndex() ) == page_reply ) {
    edit = textEdit_reply;
  } else if( toolBox1->widget( toolBox1->currentIndex() ) == page_reply_all ) {
    edit = textEdit_reply_all;
  } else if( toolBox1->widget( toolBox1->currentIndex() ) == page_forward ) {
    edit = textEdit_forward;
  } else {
    kDebug() << "Unknown current page in TemplatesConfiguration!";
    return;
  }

  // kDebug() << "Insert command:" << cmd;
  QTextCursor cursor = edit->textCursor();
  cursor.insertText( cmd );
  cursor.setPosition( cursor.position() + adjustCursor );
  edit->setTextCursor( cursor );
  edit->setFocus();
}

QString TemplatesConfiguration::strOrBlank( const QString &str ) {
  if ( str.trimmed().isEmpty() ) {
    return QString( "%BLANK" );
  }
  return str;
}

QString TemplatesConfiguration::configIdString( uint id )
{
  return QString::fromLatin1( "IDENTITY_%1" ).arg( id );
}

#include "templatesconfiguration.moc"
