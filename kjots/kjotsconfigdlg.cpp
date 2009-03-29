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
#include <kdemacros.h>


KJotsConfigDlg::KJotsConfigDlg( const QString & title, QWidget *parent )
  : KCMultiDialog( parent )
{
  setCaption( title );
  setFaceType( KPageDialog::List );
  setButtons( Default | Ok | Cancel );
  setDefaultButton( Ok );

  showButtonSeparator( true );

  addModule( "kjots_config_misc" );
  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
}


KJotsConfigDlg::~KJotsConfigDlg()
{
}

void KJotsConfigDlg::slotOk()
{
}

extern "C"
{
  KDE_EXPORT KCModule *create_kjots_config_misc( QWidget *parent )
  {
      KComponentData instance( "kjots_config_misc" );
      return new KJotsConfigMisc( instance, parent );
  }
}

KJotsConfigMisc::KJotsConfigMisc( const KComponentData &inst, QWidget *parent )
    :KCModule( inst, parent )
{
    QHBoxLayout *lay = new QHBoxLayout( this );
    QWidget * w =  new confPageMisc( 0 );
    lay->addWidget( w );
    load();
}

void KJotsConfigMisc::load()
{
    KCModule::load();
}

void KJotsConfigMisc::save()
{
    KCModule::save();
}

#include "kjotsconfigdlg.moc"
