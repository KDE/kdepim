/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
                                                                        
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qlayout.h>

#include <kaboutdata.h>
#include <kdebug.h>
#include <klocale.h>

#include "kabconfigwidget.h"

#include "kcmkabconfig.h"

extern "C"
{
  KCModule *create_kabconfig( QWidget *parent, const char * ) {
    return new KCMKabConfig( parent, "kcmkabconfig" );
  }
}

KCMKabConfig::KCMKabConfig( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  mConfigWidget = new KABConfigWidget( this, "mConfigWidget" );
  layout->addWidget( mConfigWidget );

  connect( mConfigWidget, SIGNAL( changed( bool ) ), SIGNAL( changed( bool ) ) );

  load();
  
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkabconfig" ),
                                      I18N_NOOP( "KAddressBook Configure Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c), 2003 - 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  setAboutData( about );
}

void KCMKabConfig::load()
{
  mConfigWidget->restoreSettings();
}

void KCMKabConfig::save()
{
  mConfigWidget->saveSettings();
}

void KCMKabConfig::defaults()
{
  mConfigWidget->defaults();
}

#include "kcmkabconfig.moc"
