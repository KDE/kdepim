/*
 * Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 * Copyright (C) 2012 Laurent Montel <montel@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "templatesconfiguration.h"
#include "globalsettings_base.h"
#include "templatesconfiguration_kfg.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"

#include <KMessageBox>

#include <QWhatsThis>

using namespace TemplateParser;

TemplatesConfiguration::TemplatesConfiguration( QWidget *parent, const QString &name )
    : QWidget( parent )
{
    setupUi( this );
    setObjectName( name );

    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    sizeHint();

    connect( textEdit_new->editor(), SIGNAL(textChanged()),
             this, SLOT(slotTextChanged()) );
    connect( textEdit_reply->editor(), SIGNAL(textChanged()),
             this, SLOT(slotTextChanged()) );
    connect( textEdit_reply_all->editor(), SIGNAL(textChanged()),
             this, SLOT(slotTextChanged()) );
    connect( textEdit_forward->editor(), SIGNAL(textChanged()),
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
    const QString templateConfigurationName( name );
    if ( templateConfigurationName == QLatin1String( "folder-templates" ) ) {
        mHelpString +=
                i18n( "<p>Templates specified here are folder-specific. "
                      "They override both global templates and per-identity "
                      "templates.</p>" );
    } else if ( templateConfigurationName == QLatin1String( "identity-templates" ) ) {
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
    mHelp->setContextMenuPolicy(Qt::NoContextMenu);
    connect( mHelp, SIGNAL(linkActivated(QString)),
             this, SLOT(slotHelpLinkClicked(QString)) );
}

void TemplatesConfiguration::slotHelpLinkClicked( const QString & )
{
    QWhatsThis::showText( QCursor::pos(), mHelpString );
}

void TemplatesConfiguration::slotTextChanged()
{
    emit changed();
}

void TemplatesConfiguration::resetToDefault()
{
    const int choice =
            KMessageBox::questionYesNoCancel(
                0,
                i18n( "Do you want to reset current template or all templates to default?" ),
                i18n( "Reset to default" ),
                KGuiItem( i18n( "Reset Current Template" ) ),
                KGuiItem( i18n( "Reset All Templates" ) ),
                KStandardGuiItem::cancel() );

    if ( choice == KMessageBox::Cancel ) {
        return;
    } else if ( choice == KMessageBox::Yes ) {
        const int toolboxCurrentIndex( toolBox1->currentIndex() );
        if( toolBox1->widget( toolboxCurrentIndex ) == page_new ) {
            textEdit_new->setPlainText( DefaultTemplates::defaultNewMessage() );
        } else if( toolBox1->widget( toolboxCurrentIndex ) == page_reply ) {
            textEdit_reply->setPlainText( DefaultTemplates::defaultReply() );
        } else if( toolBox1->widget( toolboxCurrentIndex ) == page_reply_all ) {
            textEdit_reply_all->setPlainText( DefaultTemplates::defaultReplyAll() );
        } else if( toolBox1->widget( toolboxCurrentIndex ) == page_forward ) {
            textEdit_forward->setPlainText( DefaultTemplates::defaultForward() );
        } else {
            kDebug() << "Unknown current page in TemplatesConfiguration!";
        }
    } else {
        textEdit_new->setPlainText( DefaultTemplates::defaultNewMessage() );
        textEdit_reply->setPlainText( DefaultTemplates::defaultReply() );
        textEdit_reply_all->setPlainText( DefaultTemplates::defaultReplyAll() );
        textEdit_forward->setPlainText( DefaultTemplates::defaultForward() );
    }
    lineEdit_quote->setText( DefaultTemplates::defaultQuoteString() );
}

void TemplatesConfiguration::loadFromGlobal()
{
    QString str;
    str = GlobalSettings::self()->templateNewMessage();
    if ( str.isEmpty() ) {
        textEdit_new->setPlainText( DefaultTemplates::defaultNewMessage() );
    } else {
        textEdit_new->setPlainText(str);
    }
    str = GlobalSettings::self()->templateReply();
    if ( str.isEmpty() ) {
        textEdit_reply->setPlainText( DefaultTemplates::defaultReply() );
    } else {
        textEdit_reply->setPlainText( str );
    }
    str = GlobalSettings::self()->templateReplyAll();
    if ( str.isEmpty() ) {
        textEdit_reply_all->setPlainText( DefaultTemplates::defaultReplyAll() );
    } else {
        textEdit_reply_all->setPlainText( str );
    }
    str = GlobalSettings::self()->templateForward();
    if ( str.isEmpty() ) {
        textEdit_forward->setPlainText( DefaultTemplates::defaultForward() );
    } else {
        textEdit_forward->setPlainText( str );
    }
    str = GlobalSettings::self()->quoteString();
    if ( str.isEmpty() ) {
        lineEdit_quote->setText( DefaultTemplates::defaultQuoteString() );
    } else {
        lineEdit_quote->setText( str );
    }
}

void TemplatesConfiguration::saveToGlobal()
{
    GlobalSettings::self()->setTemplateNewMessage( strOrBlank( textEdit_new->toPlainText() ) );
    GlobalSettings::self()->setTemplateReply( strOrBlank( textEdit_reply->toPlainText() ) );
    GlobalSettings::self()->setTemplateReplyAll( strOrBlank( textEdit_reply_all->toPlainText() ) );
    GlobalSettings::self()->setTemplateForward( strOrBlank( textEdit_forward->toPlainText() ) );
    GlobalSettings::self()->setQuoteString( lineEdit_quote->text() );
    GlobalSettings::self()->writeConfig();
}

void TemplatesConfiguration::loadFromIdentity( uint id )
{
    Templates t( configIdString( id ) );

    QString str;

    str = t.templateNewMessage();
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->templateNewMessage();
    }
    if ( str.isEmpty() ) {
        str = DefaultTemplates::defaultNewMessage();
    }
    textEdit_new->setPlainText( str );

    str = t.templateReply();
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->templateReply();
    }
    if ( str.isEmpty() ) {
        str = DefaultTemplates::defaultReply();
    }
    textEdit_reply->setPlainText( str );

    str = t.templateReplyAll();
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->templateReplyAll();
    }
    if ( str.isEmpty() ) {
        str = DefaultTemplates::defaultReplyAll();
    }
    textEdit_reply_all->setPlainText( str );

    str = t.templateForward();
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->templateForward();
    }
    if ( str.isEmpty() ) {
        str = DefaultTemplates::defaultForward();
    }
    textEdit_forward->setPlainText( str );

    str = t.quoteString();
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->quoteString();
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
    Templates *tid = 0;

    if ( identity ) {
        tid = new Templates( configIdString( identity ) );
    }

    QString str;

    str = t.templateNewMessage();
    if ( str.isEmpty() && tid ) {
        str = tid->templateNewMessage();
    }
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->templateNewMessage();
    }
    if ( str.isEmpty() ) {
        str = DefaultTemplates::defaultNewMessage();
    }
    textEdit_new->setPlainText( str );

    str = t.templateReply();
    if ( str.isEmpty() && tid ) {
        str = tid->templateReply();
    }
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->templateReply();
    }
    if ( str.isEmpty() ) {
        str = DefaultTemplates::defaultReply();
    }
    textEdit_reply->setPlainText( str );

    str = t.templateReplyAll();
    if ( str.isEmpty() && tid ) {
        str = tid->templateReplyAll();
    }
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->templateReplyAll();
    }
    if ( str.isEmpty() ) {
        str = DefaultTemplates::defaultReplyAll();
    }
    textEdit_reply_all->setPlainText( str );

    str = t.templateForward();
    if ( str.isEmpty() && tid ) {
        str = tid->templateForward();
    }
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->templateForward();
    }
    if ( str.isEmpty() ) {
        str = DefaultTemplates::defaultForward();
    }
    textEdit_forward->setPlainText( str );

    str = t.quoteString();
    if ( str.isEmpty() && tid ) {
        str = tid->quoteString();
    }
    if ( str.isEmpty() ) {
        str = GlobalSettings::self()->quoteString();
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

QTextEdit *TemplatesConfiguration::currentTextEdit() const
{
    QTextEdit *edit;

    const int toolboxCurrentIndex( toolBox1->currentIndex() );
    if( toolBox1->widget( toolboxCurrentIndex ) == page_new ) {
        edit = textEdit_new->editor();
    } else if( toolBox1->widget( toolboxCurrentIndex ) == page_reply ) {
        edit = textEdit_reply->editor();
    } else if( toolBox1->widget( toolboxCurrentIndex ) == page_reply_all ) {
        edit = textEdit_reply_all->editor();
    } else if( toolBox1->widget( toolboxCurrentIndex ) == page_forward ) {
        edit = textEdit_forward->editor();
    } else {
        kDebug() << "Unknown current page in TemplatesConfiguration!";
        edit = 0;
    }
    return edit;
}

void TemplatesConfiguration::slotInsertCommand( const QString &cmd, int adjustCursor )
{
    QTextEdit *edit = currentTextEdit();
    if ( !edit ) {
        return;
    }

    // kDebug() << "Insert command:" << cmd;
    const QString editText( edit->toPlainText() );
    if ( ( editText.contains( QLatin1String("%FORCEDPLAIN") ) && ( cmd == QLatin1String( "%FORCEDHTML" ) ) ) ||
         ( editText.contains( QLatin1String("%FORCEDHTML") ) && ( cmd == QLatin1String( "%FORCEDPLAIN" ) ) ) ) {
        KMessageBox::error(
                    this,
                    i18n( "Use of \"Reply using plain text\" and \"Reply using HTML text\" in pairs"
                          " is not correct. Use only one of the aforementioned commands with \" Reply as"
                          " Quoted Message command\" as per your need\n"
                          "(a)Reply using plain text for quotes to be strictly in plain text\n"
                          "(b)Reply using HTML text for quotes being in HTML format if present" ) );
    } else {
        QTextCursor cursor = edit->textCursor();
        cursor.insertText( cmd );
        cursor.setPosition( cursor.position() + adjustCursor );
        edit->setTextCursor( cursor );
        edit->setFocus();
    }
}

QString TemplatesConfiguration::strOrBlank( const QString &str )
{
    if ( str.trimmed().isEmpty() ) {
        return QString::fromLatin1( "%BLANK" );
    }
    return str;
}

QString TemplatesConfiguration::configIdString( uint id )
{
    return QString::fromLatin1( "IDENTITY_%1" ).arg( id );
}

