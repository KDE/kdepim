/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2005, The KNotes Developers

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************/

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqhgroupbox.h>
#include <tqtabwidget.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kfontrequester.h>
#include <kwin.h>

#include "knote.h"
#include "knoteconfigdlg.h"
#include "knotesglobalconfig.h"
#include "version.h"


KNoteConfigDlg::KNoteConfigDlg( KNoteConfig *config, const TQString& title,
        TQWidget *parent, const char *name )
    : KConfigDialog( parent, name, config ? config : KNotesGlobalConfig::self(), IconList,
                     config ? Default|Ok|Apply|Cancel : Default|Ok|Cancel, Ok )
{
    setCaption( title );
    KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );

    setIconListAllVisible( true );
    enableButtonSeparator( true );

    if ( config )
    {
        addPage( makeDisplayPage( false ), i18n("Display"), "knotes",
                 i18n("Display Settings") );
        addPage( makeEditorPage( false ), i18n("Editor"), "edit",
                 i18n("Editor Settings") );
    }
    else
    {
        config = KNotesGlobalConfig::self();
        addPage( makeDefaultsPage(), i18n("Defaults"), "knotes",
                 i18n("Default Settings for New Notes") );
        addPage( makeActionsPage(), i18n("Actions"), "misc",
                 i18n("Action Settings") );
        addPage( makeNetworkPage(), i18n("Network"), "network",
                 i18n("Network Settings") );
        addPage( makeStylePage(), i18n("Style"), "style",
                 i18n("Style Settings") );
    }

    config->setVersion( KNOTES_VERSION );
}

KNoteConfigDlg::~KNoteConfigDlg()
{
}

void KNoteConfigDlg::slotUpdateCaption()
{
    KNote *note = ::qt_cast<KNote *>(sender());
    if ( note )
        setCaption( note->name() );
}

TQWidget *KNoteConfigDlg::makeDisplayPage( bool defaults )
{
    TQWidget *displayPage = new TQWidget();
    TQGridLayout *layout = new TQGridLayout( displayPage, 2, 2,
                                           defaults ? marginHint() : 0, spacingHint() );

    TQLabel *label_FgColor = new TQLabel( i18n("&Text color:"), displayPage, "label_FgColor" );
    layout->addWidget( label_FgColor, 0, 0 );

    KColorButton *kcfg_FgColor = new KColorButton( displayPage, "kcfg_FgColor" );
    label_FgColor->setBuddy( kcfg_FgColor );
    layout->addWidget( kcfg_FgColor, 0, 1 );

    TQLabel *label_BgColor = new TQLabel( i18n("&Background color:"), displayPage, "label_BgColor" );
    layout->addWidget( label_BgColor, 1, 0 );

    KColorButton *kcfg_BgColor = new KColorButton( displayPage, "kcfg_BgColor" );
    label_BgColor->setBuddy( kcfg_BgColor );
    layout->addWidget( kcfg_BgColor, 1, 1 );

    TQCheckBox *kcfg_ShowInTaskbar = new TQCheckBox( i18n("&Show note in taskbar"),
                                                   displayPage, "kcfg_ShowInTaskbar" );

    if ( defaults )
    {
        TQLabel *label_Width = new TQLabel( i18n("Default &width:"), displayPage, "label_Width" );
        layout->addWidget( label_Width, 2, 0 );

        KIntNumInput *kcfg_Width = new KIntNumInput( displayPage, "kcfg_Width" );
        label_Width->setBuddy( kcfg_Width );
        kcfg_Width->setRange( 50, 2000, 10, false );
        layout->addWidget( kcfg_Width, 2, 1 );

        TQLabel *label_Height = new TQLabel( i18n("Default &height:"), displayPage, "label_Height" );
        layout->addWidget( label_Height, 3, 0 );

        KIntNumInput *kcfg_Height = new KIntNumInput( displayPage, "kcfg_Height" );
        kcfg_Height->setRange( 50, 2000, 10, false );
        label_Height->setBuddy( kcfg_Height );
        layout->addWidget( kcfg_Height, 3, 1 );

        layout->addWidget( kcfg_ShowInTaskbar, 4, 0 );
    }
    else
        layout->addWidget( kcfg_ShowInTaskbar, 2, 0 );

    return displayPage;
}

TQWidget *KNoteConfigDlg::makeEditorPage( bool defaults )
{
    TQWidget *editorPage = new TQWidget();
    TQGridLayout *layout = new TQGridLayout( editorPage, 4, 3,
                                           defaults ? marginHint() : 0, spacingHint() );

    TQLabel *label_TabSize = new TQLabel( i18n( "&Tab size:" ), editorPage, "label_TabSize" );
    layout->addMultiCellWidget( label_TabSize, 0, 0, 0, 1 );

    KIntNumInput *kcfg_TabSize = new KIntNumInput( editorPage, "kcfg_TabSize" );
    kcfg_TabSize->setRange( 0, 40, 1, false );
    label_TabSize->setBuddy( kcfg_TabSize );
    layout->addWidget( kcfg_TabSize, 0, 2 );

    TQCheckBox *kcfg_AutoIndent = new TQCheckBox( i18n("Auto &indent"), editorPage, "kcfg_AutoIndent" );
    layout->addMultiCellWidget( kcfg_AutoIndent, 1, 1, 0, 1 );

    TQCheckBox *kcfg_RichText = new TQCheckBox( i18n("&Rich text"), editorPage, "kcfg_RichText" );
    layout->addWidget( kcfg_RichText, 1, 2 );

    TQLabel *label_Font = new TQLabel( i18n("Text font:"), editorPage, "label_Font" );
    layout->addWidget( label_Font, 3, 0 );

    KFontRequester *kcfg_Font = new KFontRequester( editorPage, "kcfg_Font" );
    kcfg_Font->setSizePolicy( TQSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Fixed ) );
    layout->addMultiCellWidget( kcfg_Font, 3, 3, 1, 2 );

    TQLabel *label_TitleFont = new TQLabel( i18n("Title font:"), editorPage, "label_TitleFont" );
    layout->addWidget( label_TitleFont, 2, 0 );

    KFontRequester *kcfg_TitleFont = new KFontRequester( editorPage, "kcfg_TitleFont" );
    kcfg_TitleFont->setSizePolicy( TQSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Fixed ) );
    layout->addMultiCellWidget( kcfg_TitleFont, 2, 2, 1, 2 );

    return editorPage;
}

TQWidget *KNoteConfigDlg::makeDefaultsPage()
{
    TQTabWidget *defaultsPage = new TQTabWidget();
    defaultsPage->addTab( makeDisplayPage( true ), SmallIconSet( "knotes" ), i18n("Displa&y") );
    defaultsPage->addTab( makeEditorPage( true ), SmallIconSet( "edit" ), i18n("&Editor") );

    return defaultsPage;
}

TQWidget *KNoteConfigDlg::makeActionsPage()
{
    TQWidget *actionsPage = new TQWidget();
    TQGridLayout *layout = new TQGridLayout( actionsPage, 2, 2, 0, spacingHint() );

    TQLabel *label_MailAction = new TQLabel( i18n("&Mail action:"), actionsPage, "label_MailAction" );
    layout->addWidget( label_MailAction, 0, 0 );

    KLineEdit *kcfg_MailAction = new KLineEdit( actionsPage, "kcfg_MailAction" );
    label_MailAction->setBuddy( kcfg_MailAction );
    layout->addWidget( kcfg_MailAction, 0, 1 );

    return actionsPage;
}

TQWidget *KNoteConfigDlg::makeNetworkPage()
{
    TQWidget *networkPage = new TQWidget();
    TQGridLayout *layout = new TQGridLayout( networkPage, 4, 2, 0, spacingHint() );

    TQGroupBox *incoming = new TQHGroupBox( i18n("Incoming Notes"), networkPage );
    layout->addMultiCellWidget( incoming, 0, 0, 0, 1 );

    new TQCheckBox( i18n("Accept incoming notes"), incoming, "kcfg_ReceiveNotes" );

    TQGroupBox *outgoing = new TQHGroupBox( i18n("Outgoing Notes"), networkPage );
    layout->addMultiCellWidget( outgoing, 1, 1, 0, 1 );

    TQLabel *label_SenderID = new TQLabel( i18n("&Sender ID:"), outgoing, "label_SenderID" );
    KLineEdit *kcfg_SenderID = new KLineEdit( outgoing, "kcfg_SenderID" );
    label_SenderID->setBuddy( kcfg_SenderID );

    TQLabel *label_Port = new TQLabel( i18n("&Port:"), networkPage, "label_Port" );
    layout->addWidget( label_Port, 2, 0 );

    KIntNumInput *kcfg_Port = new KIntNumInput( networkPage, "kcfg_Port" );
    kcfg_Port->setRange( 0, 65535, 1, false );
    label_Port->setBuddy( kcfg_Port );
    layout->addWidget( kcfg_Port, 2, 1 );

    return networkPage;
}

TQWidget *KNoteConfigDlg::makeStylePage()
{
    TQWidget *stylePage = new TQWidget();
    TQGridLayout *layout = new TQGridLayout( stylePage, 2, 2, 0, spacingHint() );

    TQLabel *label_Style = new TQLabel( i18n("&Style:"), stylePage, "label_Style" );
    layout->addWidget( label_Style, 0, 0 );

    TQComboBox *kcfg_Style = new TQComboBox( stylePage, "kcfg_Style" );
    TQStringList list;
    list << "Plain" << "Fancy";
    kcfg_Style->insertStringList( list );
    label_Style->setBuddy( kcfg_Style );
    layout->addWidget( kcfg_Style, 0, 1 );

    return stylePage;
}

#include "knoteconfigdlg.moc"
