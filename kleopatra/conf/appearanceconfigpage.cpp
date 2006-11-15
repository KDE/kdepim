/*
    appearanceconfigpage.cpp

    This file is part of kleopatra, the KDE key manager
    Copyright (c) 2004 Klar�vdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "appearanceconfigpage.h"
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>
#include "appearanceconfigwidget.h"
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kdemacros.h>

AppearanceConfigurationPage::AppearanceConfigurationPage( KInstance *instance, QWidget *parent, const QStringList &args )
    : KCModule( instance, parent, args )
{
  QVBoxLayout* lay = new QVBoxLayout( this );
  mWidget = new Kleo::AppearanceConfigWidget( this );
  lay->addWidget( mWidget );
  connect( mWidget, SIGNAL( changed() ), this, SLOT( slotChanged() ) );

#ifndef HAVE_UNBROKEN_KCMULTIDIALOG
  load();
#endif
}


void AppearanceConfigurationPage::load()
{
  mWidget->load();
}

void AppearanceConfigurationPage::save()
{
  mWidget->save();

}

void AppearanceConfigurationPage::defaults()
{
  mWidget->defaults();
}

extern "C"
{
  KDE_EXPORT KCModule *create_kleopatra_config_appear( QWidget *parent=0, const QStringList &args=QStringList() )
  {
    AppearanceConfigurationPage *page =
      new AppearanceConfigurationPage( new KInstance( "kleopatra" ), parent, args );
    page->setObjectName( "kleopatra_config_appear" );
    return page;
  }
}

// kdelibs-3.2 didn't have the changed signal in KCModule...
void AppearanceConfigurationPage::slotChanged()
{
  emit changed(true);
}

#include "appearanceconfigpage.moc"
