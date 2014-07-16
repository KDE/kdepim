/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Tobias Koenig <tobias.koenig@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "configwidget.h"

#include "messagecomposer/settings/messagecomposersettings.h"
#include "messagelistsettings.h"
#include "messageviewer/settings/globalsettings.h"
#include "settings.h"
#include "stylesheetloader.h"
#include "templateparser/globalsettings_base.h"
#include "ui_configwidget.h"

#include <kconfigdialogmanager.h>
#include <klocale.h>
#include <libkdepim/addressline/completionordereditor.h>
#include <libkdepim/ldap/ldapclientsearch.h>
#include <libkdepim/addressline/recentaddresses.h>

#include <QLineEdit>

using namespace MessageComposer;

ConfigWidget::ConfigWidget( QWidget *parent )
  : QWidget( parent )
{
  Ui_ConfigWidget ui;
  ui.setupUi( this );

  ui.kcfg_ComposerTemplatesNewMessage->setProperty( "kcfg_property", QByteArray( "plainText" ) );
  ui.kcfg_ComposerTemplatesReplyToSender->setProperty( "kcfg_property", QByteArray( "plainText" ) );
  ui.kcfg_ComposerTemplatesReplyToAll->setProperty( "kcfg_property", QByteArray( "plainText" ) );
  ui.kcfg_ComposerTemplatesForwardMessage->setProperty( "kcfg_property", QByteArray( "plainText" ) );

  {
    QLineEdit *lineEdit = ui.kcfg_ComposerWordWrapColumn->findChild<QLineEdit*>();
    if ( lineEdit )
      lineEdit->setReadOnly( true );
  }

  mManager = new KConfigDialogManager( this, Settings::self() );

  connect( ui.configureCompletionOrderButton, SIGNAL(clicked()),
           this, SLOT(configureCompletionOrder()) );
  connect( ui.editRecentAddressesButton, SIGNAL(clicked()),
           this, SLOT(editRecentAddresses()) );

  ui.howDoesThisWorkLabel->setText( i18n( "<a href=\"help\">How does this work?</a>" ) );
  connect( ui.howDoesThisWorkLabel, SIGNAL(linkActivated(QString)),
           this, SIGNAL(showTemplatesHelp()) );

  ui.helpLabel->setVisible( false );
}

void ConfigWidget::load()
{
  loadFromExternalSettings();
  mManager->updateWidgets();
}

void ConfigWidget::save()
{
  mManager->updateSettings();
  saveToExternalSettings();

  emit configChanged();
}

void ConfigWidget::configureCompletionOrder()
{
  KLDAP::LdapClientSearch search;
  KPIM::CompletionOrderEditor editor( &search, 0 );
  editor.exec();
}

void ConfigWidget::editRecentAddresses()
{
  KPIM::RecentAddressDialog dlg( 0 );
  dlg.setAddresses( KPIM::RecentAddresses::self( MessageComposer::MessageComposerSettings::self()->config() )->addresses() );
  if ( dlg.exec() ) {
    KPIM::RecentAddresses::self( MessageComposer::MessageComposerSettings::self()->config() )->clear();
    foreach ( const QString &address, dlg.addresses() ) {
      KPIM::RecentAddresses::self( MessageComposer::MessageComposerSettings::self()->config() )->add( address );
    }
  }
}

void ConfigWidget::loadFromExternalSettings()
{
  // Appearance
  Settings::self()->setAppearanceShowHtmlStatusBar( MessageViewer::GlobalSettings::showColorBar() );
  Settings::self()->setAppearanceReplaceSmileys( MessageViewer::GlobalSettings::showEmoticons() );
  Settings::self()->setAppearanceReduceQuotedFontSize( MessageViewer::GlobalSettings::shrinkQuotes() );

  // Composer
  Settings::self()->setComposerInsertSignature( MessageComposerSettings::self()->autoTextSignature() == QLatin1String( "auto" ) );
  Settings::self()->setComposertInsertSignatureAboveQuote( MessageComposerSettings::self()->prependSignature() );
  Settings::self()->setComposerPrependSeparator( MessageComposerSettings::self()->dashDashSignature() );
  Settings::self()->setComposerUseSmartQuoting( TemplateParser::GlobalSettings::self()->smartQuote() );
  Settings::self()->setComposerUseRecentAddressCompletion( MessageComposerSettings::self()->showRecentAddressesInComposer() );
  Settings::self()->setComposerWordWrapAtColumn( MessageComposerSettings::self()->wordWrap() );
  Settings::self()->setComposerWordWrapColumn( MessageComposerSettings::self()->lineWrapWidth() );
  Settings::self()->setComposerReplaceReplyPrefixes( MessageComposerSettings::self()->replaceReplyPrefix() );
  Settings::self()->setComposerReplaceForwardPrefixes( MessageComposerSettings::self()->replaceForwardPrefix() );
  Settings::self()->setComposerOutlookCompatibleNaming( MessageComposerSettings::self()->outlookCompatibleAttachments() );
  Settings::self()->setComposerTemplatesNewMessage( TemplateParser::GlobalSettings::self()->templateNewMessage() );
  Settings::self()->setComposerTemplatesReplyToSender( TemplateParser::GlobalSettings::self()->templateReply() );
  Settings::self()->setComposerTemplatesReplyToAll( TemplateParser::GlobalSettings::self()->templateReplyAll() );
  Settings::self()->setComposerTemplatesForwardMessage( TemplateParser::GlobalSettings::self()->templateForward() );

  // Invitations
  Settings::self()->setInvitationsOutlookCompatible( MessageViewer::GlobalSettings::self()->legacyMangleFromToHeaders() &&
                                                     MessageViewer::GlobalSettings::self()->legacyBodyInvites() &&
                                                     MessageViewer::GlobalSettings::self()->exchangeCompatibleInvitations() &&
                                                     MessageViewer::GlobalSettings::self()->outlookCompatibleInvitationReplyComments() &&
                                                     MessageViewer::GlobalSettings::self()->outlookCompatibleInvitationComparisons() );
  Settings::self()->setInvitationsAutomaticSending( MessageViewer::GlobalSettings::self()->automaticSending() );
  Settings::self()->setInvitationsDeleteAfterReply( MessageViewer::GlobalSettings::self()->deleteInvitationEmailsAfterSendingReply() );

  // MDN
  Settings::self()->setMDNPolicy( MessageViewer::GlobalSettings::self()->defaultPolicy() );
  Settings::self()->setMDNQuoteType( MessageViewer::GlobalSettings::self()->quoteMessage() );
}

void ConfigWidget::saveToExternalSettings()
{
  // Appearance
  MessageViewer::GlobalSettings::self()->setShowColorBar( Settings::self()->appearanceShowHtmlStatusBar() );
  MessageViewer::GlobalSettings::self()->setShowEmoticons( Settings::self()->appearanceReplaceSmileys() );
  MessageViewer::GlobalSettings::self()->setShrinkQuotes( Settings::self()->appearanceReduceQuotedFontSize() );

  // Composer
  MessageComposerSettings::self()->setAutoTextSignature( Settings::self()->composerInsertSignature() ? QLatin1String("auto") : QLatin1String("manual") );
  MessageComposerSettings::self()->setPrependSignature( Settings::self()->composertInsertSignatureAboveQuote() );
  MessageComposerSettings::self()->setDashDashSignature( Settings::self()->composerPrependSeparator() );
  TemplateParser::GlobalSettings::self()->setSmartQuote( Settings::self()->composerUseSmartQuoting() );
  MessageComposerSettings::self()->setShowRecentAddressesInComposer( Settings::self()->composerUseRecentAddressCompletion() );
  MessageComposerSettings::self()->setWordWrap( Settings::self()->composerWordWrapAtColumn() );
  MessageComposerSettings::self()->setLineWrapWidth( Settings::self()->composerWordWrapColumn() );
  MessageComposerSettings::self()->setReplaceReplyPrefix( Settings::self()->composerReplaceReplyPrefixes() );
  MessageComposerSettings::self()->setReplaceForwardPrefix( Settings::self()->composerReplaceForwardPrefixes() );
  MessageComposerSettings::self()->setOutlookCompatibleAttachments( Settings::self()->composerOutlookCompatibleNaming() );
  TemplateParser::GlobalSettings::self()->setTemplateNewMessage( Settings::self()->composerTemplatesNewMessage() );
  TemplateParser::GlobalSettings::self()->setTemplateReply( Settings::self()->composerTemplatesReplyToSender() );
  TemplateParser::GlobalSettings::self()->setTemplateReplyAll( Settings::self()->composerTemplatesReplyToAll() );
  TemplateParser::GlobalSettings::self()->setTemplateForward( Settings::self()->composerTemplatesForwardMessage() );

  // Invitations
  MessageViewer::GlobalSettings::self()->setLegacyMangleFromToHeaders( Settings::self()->invitationsOutlookCompatible() );
  MessageViewer::GlobalSettings::self()->setLegacyBodyInvites( Settings::self()->invitationsOutlookCompatible() );
  MessageViewer::GlobalSettings::self()->setExchangeCompatibleInvitations( Settings::self()->invitationsOutlookCompatible() );
  MessageViewer::GlobalSettings::self()->setOutlookCompatibleInvitationReplyComments( Settings::self()->invitationsOutlookCompatible() );
  MessageViewer::GlobalSettings::self()->setOutlookCompatibleInvitationComparisons( Settings::self()->invitationsOutlookCompatible() );
  MessageViewer::GlobalSettings::self()->setAutomaticSending( Settings::self()->invitationsAutomaticSending() );
  MessageViewer::GlobalSettings::self()->setDeleteInvitationEmailsAfterSendingReply( Settings::self()->invitationsDeleteAfterReply() );

  // MDN
  MessageViewer::GlobalSettings::self()->setDefaultPolicy( Settings::self()->mDNPolicy() );
  MessageViewer::GlobalSettings::self()->setQuoteMessage( Settings::self()->mDNQuoteType() );

  Settings::self()->writeConfig();
  MessageViewer::GlobalSettings::self()->writeConfig();
  TemplateParser::GlobalSettings::self()->writeConfig();
}


DeclarativeConfigWidget::DeclarativeConfigWidget( QGraphicsItem *parent )
  : DeclarativeWidgetBase<ConfigWidget, MainView, &MainView::setConfigWidget>( parent )
{
  connect( this, SIGNAL(configChanged()), widget(), SIGNAL(configChanged()) );
}

DeclarativeConfigWidget::~DeclarativeConfigWidget()
{
}

void DeclarativeConfigWidget::load()
{
  widget()->load();
}

void DeclarativeConfigWidget::save()
{
  widget()->save();
}

