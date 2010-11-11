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

#include "messagecomposer/messagecomposersettings.h"
#include "messageviewer/globalsettings.h"
#include "settings.h"
#include "stylesheetloader.h"
#include "templateparser/globalsettings_base.h"
#include "ui_configwidget.h"

#include <kconfigdialogmanager.h>
#include <klocale.h>
#include <libkdepim/completionordereditor.h>
#include <libkdepim/ldap/ldapclient.h>
#include <libkdepim/recentaddresses.h>

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

  mManager = new KConfigDialogManager( this, Settings::self() );

  connect( ui.configureCompletionOrderButton, SIGNAL( clicked() ),
           this, SLOT( configureCompletionOrder() ) );
  connect( ui.editRecentAddressesButton, SIGNAL( clicked() ),
           this, SLOT( editRecentAddresses() ) );

  ui.howDoesThisWorkLabel->setText( i18n( "<a href=\"help\">How does this work?</a>" ) );
  connect( ui.howDoesThisWorkLabel, SIGNAL( linkActivated( const QString& ) ),
           this, SLOT( showTemplatesHelpClicked() ) );

  mHelpLabel = ui.helpLabel;
  mHelpLabel->setVisible( false );

  QString helpText;
  helpText += i18n( "<p>Here you can create and manage templates to use when<br/>"
                    "composing new messages, replies or forwarded messages.</p>"
                    "<p>The message templates support substitution commands,<br/>"
                    "which can be used together with custom text inside the above text fields.</p>" );

  helpText += "<br/>";
  helpText += "<table>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\">" + i18n( "Original Message" ) + "</th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\"></th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\">" + i18n( "Command" ) + "</th>";
  helpText += "    <th align=\"left\">" + i18n( "Description" ) + "</th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%QUOTE</td>";
  helpText += "    <td>" + i18n( "Quoted Message Text" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%TEXT</td>";
  helpText += "    <td>" + i18n( "Message Text as Is" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OMSGID</td>";
  helpText += "    <td>" + i18n( "Message Id" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%ODATE</td>";
  helpText += "    <td>" + i18n( "Date" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%ODATESHORT</td>";
  helpText += "    <td>" + i18n( "Date in Short Format" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%ODATEEN</td>";
  helpText += "    <td>" + i18n( "Date in C Locale" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%ODOW</td>";
  helpText += "    <td>" + i18n( "Day of Week" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OTIME</td>";
  helpText += "    <td>" + i18n( "Time" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OTIMELONG</td>";
  helpText += "    <td>" + i18n( "Time in Long Format" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OTIMELONGEN</td>";
  helpText += "    <td>" + i18n( "Time in C Locale" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OTOADDR</td>";
  helpText += "    <td>" + i18n( "To Field Address" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OTONAME</td>";
  helpText += "    <td>" + i18n( "To Field Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OTOFNAME</td>";
  helpText += "    <td>" + i18n( "To Field First Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OTOLNAME</td>";
  helpText += "    <td>" + i18n( "To Field Last Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OCCADDR</td>";
  helpText += "    <td>" + i18n( "CC Field Address" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OCCNAME</td>";
  helpText += "    <td>" + i18n( "CC Field Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OCCFNAME</td>";
  helpText += "    <td>" + i18n( "CC Field First Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OCCLNAME</td>";
  helpText += "    <td>" + i18n( "CC Field Last Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OFROMADDR</td>";
  helpText += "    <td>" + i18n( "From Field Address" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OFROMNAME</td>";
  helpText += "    <td>" + i18n( "From Field Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OFROMFNAME</td>";
  helpText += "    <td>" + i18n( "From Field First Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OFROMLNAME</td>";
  helpText += "    <td>" + i18n( "From Field Last Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OADDRESSEESADDR</td>";
  helpText += "    <td>" + i18n( "Addresses of all recipients" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OFULLSUBJECT</td>";
  helpText += "    <td>" + i18n( "Subject" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%QHEADERS</td>";
  helpText += "    <td>" + i18n( "Quoted Headers" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%HEADERS</td>";
  helpText += "    <td>" + i18n( "Headers as Is" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%OHEADER=\"<i>header-name</i>\"</td>";
  helpText += "    <td>" + i18n( "Header Content" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\"></th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\">" + i18n( "Current Message" ) + "</th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\"></th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\">" + i18n( "Command" ) + "</th>";
  helpText += "    <th align=\"left\">" + i18n( "Description" ) + "</th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%MSGID</td>";
  helpText += "    <td>" + i18n( "Message Id" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%DATE</td>";
  helpText += "    <td>" + i18n( "Date" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%DATESHORT</td>";
  helpText += "    <td>" + i18n( "Date in Short Format" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%DATEEN</td>";
  helpText += "    <td>" + i18n( "Date in C Locale" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%DOW</td>";
  helpText += "    <td>" + i18n( "Day of Week" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%TIME</td>";
  helpText += "    <td>" + i18n( "Time" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%TIMELONG</td>";
  helpText += "    <td>" + i18n( "Time in Long Format" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%TIMELONGEN</td>";
  helpText += "    <td>" + i18n( "Time in C Locale" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%TOADDR</td>";
  helpText += "    <td>" + i18n( "To Field Address" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%TONAME</td>";
  helpText += "    <td>" + i18n( "To Field Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%TOFNAME</td>";
  helpText += "    <td>" + i18n( "To Field First Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%TOLNAME</td>";
  helpText += "    <td>" + i18n( "To Field Last Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%CCADDR</td>";
  helpText += "    <td>" + i18n( "CC Field Address" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%CCNAME</td>";
  helpText += "    <td>" + i18n( "CC Field Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%CCFNAME</td>";
  helpText += "    <td>" + i18n( "CC Field First Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%CCLNAME</td>";
  helpText += "    <td>" + i18n( "CC Field Last Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%FROMADDR</td>";
  helpText += "    <td>" + i18n( "From Field Address" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%FROMNAME</td>";
  helpText += "    <td>" + i18n( "From Field Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%FROMFNAME</td>";
  helpText += "    <td>" + i18n( "From Field First Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%FROMLNAME</td>";
  helpText += "    <td>" + i18n( "From Field Last Name" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%FULLSUBJECT</td>";
  helpText += "    <td>" + i18n( "Subject" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%HEADER=\"<i>header-name</i>\"</td>";
  helpText += "    <td>" + i18n( "Header Content" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\"></th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\">" + i18n( "Process with External Programm" ) + "</th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\"></th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\">" + i18n( "Command" ) + "</th>";
  helpText += "    <th align=\"left\">" + i18n( "Description" ) + "</th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%SYSTEM=\"<i>command</i>\"</td>";
  helpText += "    <td>" + i18n( "Arbitrary Command" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%QUOTEPIPE=\"<i>command</i>\"</td>";
  helpText += "    <td>" + i18n( "Pipe Original Message Body and Insert Result as Quoted Text" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%TEXTPIPE=\"<i>command</i>\"</td>";
  helpText += "    <td>" + i18n( "Pipe Original Message Body and Insert Result as Is" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%MSGPIPE=\"<i>command</i>\"</td>";
  helpText += "    <td>" + i18n( "Pipe Original Message with Headers and Insert Result as Is" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%BODYPIPE=\"<i>command</i>\"</td>";
  helpText += "    <td>" + i18n( "Pipe Current Message Body and Insert Result as Is" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%CLEARPIPE=\"<i>command</i>\"</td>";
  helpText += "    <td>" + i18n( "Pipe Current Message Body and Replace with Result" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\"></th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\">" + i18n( "Miscellaneous" ) + "</th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\" colspan=\"2\"></th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <th align=\"left\">" + i18n( "Command" ) + "</th>";
  helpText += "    <th align=\"left\">" + i18n( "Description" ) + "</th>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%SIGNATURE</td>"; 
  helpText += "    <td>" + i18n( "Signature" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%INSERT=\"<i>file name</i>\"</td>";
  helpText += "    <td>" + i18n( "Insert File Content" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%-</td>";
  helpText += "    <td>" + i18n( "Discard to Next Line" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%REM=\"<i>comment</i>\"</td>";
  helpText += "    <td>" + i18n( "Template Comment" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%NOP</td>";
  helpText += "    <td>" + i18n( "No Operation" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%CLEAR</td>";
  helpText += "    <td>" + i18n( "Clear Generated Message" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%DEBUG</td>";
  helpText += "    <td>" + i18n( "Turn Debug On" ) + "</td>";
  helpText += "  </tr>";
  helpText += "  <tr>";
  helpText += "    <td>%DEBUGOFF</td>";
  helpText += "    <td>" + i18n( "Turn Debug Off" ) + "</td>";
  helpText += "  </tr>";
  helpText += "</table>";

  mHelpLabel->setText( helpText );
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

void ConfigWidget::showTemplatesHelpClicked()
{
  mHelpLabel->setVisible( !mHelpLabel->isVisible() );
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
}

void ConfigWidget::saveToExternalSettings()
{
  // Appearance
  MessageViewer::GlobalSettings::self()->setShowColorBar( Settings::self()->appearanceShowHtmlStatusBar() );
  MessageViewer::GlobalSettings::self()->setShowEmoticons( Settings::self()->appearanceReplaceSmileys() );
  MessageViewer::GlobalSettings::self()->setShrinkQuotes( Settings::self()->appearanceReduceQuotedFontSize() );

  // Composer
  MessageComposerSettings::self()->setAutoTextSignature( Settings::self()->composerInsertSignature() ? "auto" : "manual" );
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

  Settings::self()->writeConfig();
  MessageViewer::GlobalSettings::self()->writeConfig();
  TemplateParser::GlobalSettings::self()->writeConfig();
}


DeclarativeConfigWidget::DeclarativeConfigWidget( QGraphicsItem *parent )
  : DeclarativeWidgetBase<ConfigWidget, MainView, &MainView::setConfigWidget>( parent )
{
  connect( this, SIGNAL( configChanged() ), widget(), SIGNAL( configChanged() ) );
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

#include "configwidget.moc"
