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

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtabwidget.h>
//Added by qt3to4:
#include <QGridLayout>
#include <Q3GroupBox>

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


KNoteConfigDlg::KNoteConfigDlg( KNoteConfig *config, const QString& title,
        QWidget *parent, const QString& name )
    : KConfigDialog( parent, name, config ? config : KNotesGlobalConfig::self(), IconList,
                     config ? Default|Ok|Apply|Cancel : Default|Ok|Cancel, Ok )
{
    setCaption( title );
    KWin::setIcons( winId(), qApp->windowIcon().pixmap( IconSize(K3Icon::Desktop),IconSize(K3Icon::Desktop) ), 
                             qApp->windowIcon().pixmap( IconSize(K3Icon::Small),IconSize(K3Icon::Small) ) );

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
    KNote *note = ::qobject_cast<KNote *>(sender());
    if ( note )
        setCaption( note->name() );
}

QWidget *KNoteConfigDlg::makeDisplayPage( bool defaults )
{
    QWidget *displayPage = new QWidget();
    QGridLayout *layout = new QGridLayout( displayPage );
    layout->setSpacing( spacingHint() );
    layout->setMargin( defaults ? marginHint() : 0 );

    QLabel *label_FgColor = new QLabel( i18n("&Text color:"), displayPage, "label_FgColor" );
    layout->addWidget( label_FgColor, 0, 0 );

    KColorButton *kcfg_FgColor = new KColorButton( displayPage );
    kcfg_FgColor->setObjectName( "kcfg_FgColor" );
    label_FgColor->setBuddy( kcfg_FgColor );
    layout->addWidget( kcfg_FgColor, 0, 1 );

    QLabel *label_BgColor = new QLabel( i18n("&Background color:"), displayPage, "label_BgColor" );
    layout->addWidget( label_BgColor, 1, 0 );

    KColorButton *kcfg_BgColor = new KColorButton( displayPage );
    kcfg_BgColor->setObjectName( "kcfg_BgColor" );
    label_BgColor->setBuddy( kcfg_BgColor );
    layout->addWidget( kcfg_BgColor, 1, 1 );

    QCheckBox *kcfg_ShowInTaskbar = new QCheckBox( i18n("&Show note in taskbar"),
                                                   displayPage, "kcfg_ShowInTaskbar" );

    if ( defaults )
    {
        QLabel *label_Width = new QLabel( i18n("Default &width:"), displayPage, "label_Width" );
        layout->addWidget( label_Width, 2, 0 );

        KIntNumInput *kcfg_Width = new KIntNumInput( displayPage );
        kcfg_Width->setObjectName( "kcfg_Width" );
        label_Width->setBuddy( kcfg_Width );
        kcfg_Width->setRange( 50, 2000, 10, false );
        layout->addWidget( kcfg_Width, 2, 1 );

        QLabel *label_Height = new QLabel( i18n("Default &height:"), displayPage, "label_Height" );
        layout->addWidget( label_Height, 3, 0 );

        KIntNumInput *kcfg_Height = new KIntNumInput( displayPage );
        kcfg_Height->setObjectName( "kcfg_Height" );
        kcfg_Height->setRange( 50, 2000, 10, false );
        label_Height->setBuddy( kcfg_Height );
        layout->addWidget( kcfg_Height, 3, 1 );

        layout->addWidget( kcfg_ShowInTaskbar, 4, 0 );
    }
    else
        layout->addWidget( kcfg_ShowInTaskbar, 2, 0 );

    return displayPage;
}

QWidget *KNoteConfigDlg::makeEditorPage( bool defaults )
{
    QWidget *editorPage = new QWidget();
    QGridLayout *layout = new QGridLayout( editorPage );
    layout->setSpacing( spacingHint() );
    layout->setMargin( defaults ? marginHint() : 0 );

    QLabel *label_TabSize = new QLabel( i18n( "&Tab size:" ), editorPage, "label_TabSize" );
    layout->addWidget( label_TabSize, 0, 0, 1, 2 );

    KIntNumInput *kcfg_TabSize = new KIntNumInput( editorPage );
    kcfg_TabSize->setObjectName( "kcfg_TabSize" );
    kcfg_TabSize->setRange( 0, 40, 1, false );
    label_TabSize->setBuddy( kcfg_TabSize );
    layout->addWidget( kcfg_TabSize, 0, 2 );

    QCheckBox *kcfg_AutoIndent = new QCheckBox( i18n("Auto &indent"), editorPage, "kcfg_AutoIndent" );
    layout->addWidget( kcfg_AutoIndent, 1, 0, 1, 2 );

    QCheckBox *kcfg_RichText = new QCheckBox( i18n("&Rich text"), editorPage, "kcfg_RichText" );
    layout->addWidget( kcfg_RichText, 1, 2 );

    QLabel *label_Font = new QLabel( i18n("Text font:"), editorPage, "label_Font" );
    layout->addWidget( label_Font, 3, 0 );

    KFontRequester *kcfg_Font = new KFontRequester( editorPage, "kcfg_Font" );
    kcfg_Font->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    layout->addWidget( kcfg_Font, 3, 1, 1, 2 );

    QLabel *label_TitleFont = new QLabel( i18n("Title font:"), editorPage, "label_TitleFont" );
    layout->addWidget( label_TitleFont, 2, 0 );

    KFontRequester *kcfg_TitleFont = new KFontRequester( editorPage, "kcfg_TitleFont" );
    kcfg_TitleFont->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    layout->addWidget( kcfg_TitleFont, 2, 1, 1, 2 );

    return editorPage;
}

QWidget *KNoteConfigDlg::makeDefaultsPage()
{
    QTabWidget *defaultsPage = new QTabWidget();
    defaultsPage->addTab( makeDisplayPage( true ), SmallIconSet( "knotes" ), i18n("Displa&y") );
    defaultsPage->addTab( makeEditorPage( true ), SmallIconSet( "edit" ), i18n("&Editor") );

    return defaultsPage;
}

QWidget *KNoteConfigDlg::makeActionsPage()
{
    QWidget *actionsPage = new QWidget();
    QGridLayout *layout = new QGridLayout( actionsPage );
    layout->setSpacing( spacingHint() );
    layout->setMargin( 0 );

    QLabel *label_MailAction = new QLabel( i18n("&Mail action:"), actionsPage, "label_MailAction" );
    layout->addWidget( label_MailAction, 0, 0 );

    KLineEdit *kcfg_MailAction = new KLineEdit( actionsPage );
    kcfg_MailAction->setObjectName( "kcfg_MailAction" );
    label_MailAction->setBuddy( kcfg_MailAction );
    layout->addWidget( kcfg_MailAction, 0, 1 );

    return actionsPage;
}

QWidget *KNoteConfigDlg::makeNetworkPage()
{
    QWidget *networkPage = new QWidget();
    QGridLayout *layout = new QGridLayout( networkPage );
    layout->setSpacing( spacingHint() );
    layout->setMargin( 0 );

    Q3GroupBox *incoming = new Q3GroupBox( i18n("Incoming Notes"), networkPage );
    layout->addWidget( incoming, 0, 0, 1, 2 );

    new QCheckBox( i18n("Accept incoming notes"), incoming, "kcfg_ReceiveNotes" );

    Q3GroupBox *outgoing = new Q3GroupBox( i18n("Outgoing Notes"), networkPage );
    layout->addWidget( outgoing, 1, 0, 1, 2 );

    QLabel *label_SenderID = new QLabel( i18n("&Sender ID:"), outgoing, "label_SenderID" );
    KLineEdit *kcfg_SenderID = new KLineEdit( outgoing );
    kcfg_SenderID->setObjectName( "kcfg_SenderID" );
    label_SenderID->setBuddy( kcfg_SenderID );

    QLabel *label_Port = new QLabel( i18n("&Port:"), networkPage, "label_Port" );
    layout->addWidget( label_Port, 2, 0 );

    KIntNumInput *kcfg_Port = new KIntNumInput( networkPage );
    kcfg_Port->setObjectName( "kcfg_Port" );
    kcfg_Port->setRange( 0, 65535, 1, false );
    label_Port->setBuddy( kcfg_Port );
    layout->addWidget( kcfg_Port, 2, 1 );

    return networkPage;
}

QWidget *KNoteConfigDlg::makeStylePage()
{
    QWidget *stylePage = new QWidget();
    QGridLayout *layout = new QGridLayout( stylePage );
    layout->setSpacing( spacingHint() );
    layout->setMargin( 0 );

    QLabel *label_Style = new QLabel( i18n("&Style:"), stylePage, "label_Style" );
    layout->addWidget( label_Style, 0, 0 );

    QComboBox *kcfg_Style = new QComboBox( stylePage );
    kcfg_Style->setObjectName( "kcfg_Style" );
    QStringList list;
    list << "Plain" << "Fancy";
    kcfg_Style->addItems( list );
    label_Style->setBuddy( kcfg_Style );
    layout->addWidget( kcfg_Style, 0, 1 );

    return stylePage;
}

#include "knoteconfigdlg.moc"
