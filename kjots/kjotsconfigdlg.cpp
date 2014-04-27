/*
    Copyright (c) 2009 Montel Laurent <montel@kde.org>

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
*/

#include "kjotsconfigdlg.h"
KJotsConfigDlg::KJotsConfigDlg( const QString & title, QWidget *parent )
  : KCMultiDialog( parent )
{
    //QT5 setCaption( title );
    setFaceType( KPageDialog::List );
    //QT5 setButtons( Default | Ok | Cancel );
    //QT5 setDefaultButton( Ok );

    //QT5 showButtonSeparator( true );

    addModule( QLatin1String("kjots_config_misc") );
    connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
}

KJotsConfigDlg::~KJotsConfigDlg()
{
}

void KJotsConfigDlg::slotOk()
{
}

KJotsConfigMisc::KJotsConfigMisc( const KComponentData &inst, QWidget *parent )
    :KCModule( /*inst,*/ parent )
{
    QHBoxLayout *lay = new QHBoxLayout( this );
    miscPage = new confPageMisc( 0 );
    lay->addWidget( miscPage );
    connect( miscPage->autoSaveInterval, SIGNAL(valueChanged(int)), this, SLOT(modified()) );
    connect( miscPage->autoSave, SIGNAL(stateChanged(int)), this, SLOT(modified()) );
    load();
}

void KJotsConfigMisc::modified()
{
  emit changed( true );
}

void KJotsConfigMisc::load()
{
    KConfig config( QLatin1String("kjotsrc") );
    KConfigGroup group = config.group( "kjots" );
    miscPage->autoSaveInterval->setValue( group.readEntry( "AutoSaveInterval", 5 ) );
    miscPage->autoSave->setChecked( group.readEntry( "AutoSave", true ) );
    emit changed( false );
}

void KJotsConfigMisc::save()
{
    KConfig config( QLatin1String("kjotsrc") );
    KConfigGroup group = config.group( "kjots" );
    group.writeEntry( "AutoSaveInterval", miscPage->autoSaveInterval->value() );
    group.writeEntry( "AutoSave", miscPage->autoSave->isChecked() );
    group.sync();
    emit changed( false );
}

#include "moc_kjotsconfigdlg.cpp"
