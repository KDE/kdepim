// -*- C++ -*-

//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "KJotsMain.h"
#include "aboutdata.h"

// if building kontact plugin:
#include <libkdepim/pimapplication.h>
// #else
// #include <kuniqueapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfiggroup.h>

int main( int argc, char **argv )
{
    AboutData aboutData;
    KCmdLineArgs::init(argc, argv, &aboutData);

    // if building kontact plugin:
    KPIM::PimApplication::addCmdLineOptions();
    if (!KPIM::PimApplication::start()) {
        kWarning() << "kjots is already running!";
        exit(0);
    }
    KPIM::PimApplication a;
    // else:
    //
//     KUniqueApplication::addCmdLineOptions();
//     if (!KUniqueApplication::start()) {
//         fprintf(stderr, "kjots is already running!\n");
//         exit(0);
//     }
//     KUniqueApplication a;



    // backwards compatibility code to convert "old" user font settings
    // to the new config settings
    KConfigGroup config(KGlobal::config(), "kjots");
    if (config.hasKey("EFontFamily")) {
      // read old font and create it
      QFont font( config.readEntry("EFontFamily"),
                  config.readEntry("EFontSize", 12),
                  config.readEntry("EFontWeight", 0),
                  config.readEntry("EFontItalic", 0));
      // delete old entries
      config.deleteEntry("EFontFamily");
      config.deleteEntry("EFontSize");
      config.deleteEntry("EFontWeight");
      config.deleteEntry("EFontItalic");
      config.deleteEntry("EFontCharset");
      // write new "converted" entry
      config.writeEntry("Font", font);
    }

    KJotsMain *jots = new KJotsMain;
    if( a.isSessionRestored() ) {
        if( KJotsMain::canBeRestored(1) ) {
            jots->restore(1);
        }
    }

    jots->show();
    jots->resize(jots->size());
    return a.exec();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
