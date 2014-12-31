/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "notenetworkconfig.h"
#include "settings/globalsettings.h"

#include "notesharedglobalconfig.h"

#include <KLineEdit>
#include <KComponentData>
#include <KLocalizedString>
#include <KDialog>
#include <KIntNumInput>

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QWhatsThis>
#include <QGroupBox>

using namespace NoteShared;

NoteNetworkConfigWidget::NoteNetworkConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout( this );
    QWidget * w =  new QWidget( this );
    lay->addWidget( w );
    QVBoxLayout *layout = new QVBoxLayout( w );
    layout->setSpacing( KDialog::spacingHint() );
    layout->setMargin( 0 );

    QGroupBox *incoming = new QGroupBox( i18n( "Incoming Notes" ) );
    QHBoxLayout *tmpLayout = new QHBoxLayout;

    mTmpChkB=new QCheckBox( i18n( "Accept incoming notes" ) );
    mTmpChkB->setObjectName( QLatin1String("kcfg_ReceiveNotes") );
    tmpLayout->addWidget( mTmpChkB );
    incoming->setLayout( tmpLayout );
    layout->addWidget( incoming );

    QGroupBox *outgoing = new QGroupBox( i18n( "Outgoing Notes" ) );
    tmpLayout = new QHBoxLayout;

    QLabel *label_SenderID = new QLabel( i18n( "&Sender ID:" ) );
    kcfg_SenderID = new KLineEdit;
    kcfg_SenderID->setClearButtonShown(true);
    kcfg_SenderID->setObjectName( QLatin1String("kcfg_SenderID") );
    label_SenderID->setBuddy( kcfg_SenderID );
    tmpLayout->addWidget( label_SenderID );
    tmpLayout->addWidget( kcfg_SenderID );
    outgoing->setLayout( tmpLayout );
    layout->addWidget( outgoing );

    tmpLayout = new QHBoxLayout;

    QLabel *label_Port = new QLabel( i18n( "&Port:" ) );

    tmpLayout->addWidget( label_Port );

    kcfg_Port = new KIntNumInput;
    kcfg_Port->setObjectName( QLatin1String("kcfg_Port") );
    kcfg_Port->setRange( 0, 65535 );
    kcfg_Port->setSliderEnabled( false );
    label_Port->setBuddy( kcfg_Port );
    tmpLayout->addWidget( kcfg_Port );
    layout->addLayout( tmpLayout );
    lay->addStretch();
}

NoteNetworkConfigWidget::~NoteNetworkConfigWidget()
{

}

void NoteNetworkConfigWidget::save()
{
    NoteShared::NoteSharedGlobalConfig::self()->setReceiveNotes(mTmpChkB->isChecked());
    NoteShared::NoteSharedGlobalConfig::self()->setSenderID(kcfg_SenderID->text());
    NoteShared::NoteSharedGlobalConfig::self()->setPort(kcfg_Port->value());
    NoteShared::GlobalSettings::self()->requestSync();
}

void NoteNetworkConfigWidget::load()
{
    mTmpChkB->setChecked(NoteShared::NoteSharedGlobalConfig::self()->receiveNotes());
    kcfg_SenderID->setText(NoteShared::NoteSharedGlobalConfig::self()->senderID());
    kcfg_Port->setValue(NoteShared::NoteSharedGlobalConfig::self()->port());
}


NoteNetworkConfig::NoteNetworkConfig(const KComponentData &inst, QWidget *parent )
    :KCModule( inst, parent )
{
    QVBoxLayout *lay = new QVBoxLayout( this );
    lay->setMargin(0);
    NoteNetworkConfigWidget *widget = new NoteNetworkConfigWidget(this);
    lay->addWidget( widget );
    addConfig( NoteShared::NoteSharedGlobalConfig::self(), widget );
    load();
}

void NoteNetworkConfig::save()
{
    KCModule::save();
}

void NoteNetworkConfig::load()
{
    KCModule::load();
}

