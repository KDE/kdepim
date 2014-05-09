/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include <K4AboutData>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>

#include <libkpgp/kpgp.h>
#include <libkpgp/kpgpui.h> 

int main(int argc,char **argv)
{
  K4AboutData aboutData( "testkeyselectiondialog", 0, KLocalizedString(), "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  Kpgp::KeySelectionDialog dlg( Kpgp::Module::getKpgp()->publicKeys(), QLatin1String("Public Keys") );
  dlg.show();  
  app.exec();
}
