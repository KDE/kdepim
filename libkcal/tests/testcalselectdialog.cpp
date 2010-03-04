/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "calselectdialog.h"
using namespace KCal;

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "testcalselectdialog", 0,
                      "KCalSelectDialogTest", "1.0",
                      "kcalselectedialog test app" );
  KApplication app;
  QStringList cals;
  cals << "standard" << "shared" << "mine" << "yours";
  QString cal = CalSelectDialog::getItem( i18n( "Calendar Selection" ),
                                          i18n( "Please select a calendar" ),
                                          cals );

  if ( !cal.isEmpty() ) {
    kdDebug() << "Selected calendar " << cal << endl;
  } else {
    kdDebug() << "nothing selected. user cancel" << endl;
  }
}
