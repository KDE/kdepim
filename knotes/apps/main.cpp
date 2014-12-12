/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2014, The KNotes Developers

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

#include "config-kdepim.h"
#include "knotes_debug.h"
#include "kdepim-version.h"

#include "knotes_options.h"
#include "apps/application.h"
#include <KUniqueApplication>
#include <kcmdlineargs.h>
#include <k4aboutdata.h>
#include <KLocalizedString>
#include <kxerrorhandler.h>
#include <kdelibs4configmigrator.h>

#if KDEPIM_HAVE_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <qx11info_x11.h>
#endif

void remove_sm_from_client_leader();
KCmdLineOptions knotesOptions();
void knotesAuthors(K4AboutData &aboutData);

int main(int argc, char *argv[])
{
    Kdelibs4ConfigMigrator migrate(QStringLiteral("knotes"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("knotesrc"));
    migrate.migrate();

    K4AboutData aboutData("knotes",
                          0,
                          ki18n("KNotes"),
                          KDEPIM_VERSION,
                          ki18n("KDE Notes"),
                          K4AboutData::License_GPL,
                          ki18n("Copyright © 1997–2014 KNotes authors"));

    knotesAuthors(aboutData);

    KCmdLineArgs::init(argc, argv, &aboutData);

    // Command line options

    KCmdLineArgs::addCmdLineOptions(knotesOptions());

    KUniqueApplication::addCmdLineOptions();

    if (!Application::start()) {
        qCDebug(KNOTES_LOG) << " knotes already started";
        return 0;
    }

    // Create Application

    Application app;

    remove_sm_from_client_leader();

    return app.exec();
}

void remove_sm_from_client_leader()
{
#if KDEPIM_HAVE_X11
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;

    Atom atoms[ 2 ];
    char *atom_names[ 2 ] = { (char *) "WM_CLIENT_LEADER",
                              (char *) "SM_CLIENT_ID"
                            };

    XInternAtoms(QX11Info::display(), atom_names, 2, False, atoms);

    QWidget w;
    KXErrorHandler handler; // ignore X errors
    status = XGetWindowProperty(QX11Info::display(), w.winId(), atoms[ 0 ], 0,
                                10000, false, XA_WINDOW, &type, &format, &nitems,
                                &extra, &data);

    if ((status == Success) && !handler.error(false)) {
        if (data && (nitems > 0)) {
            Window leader = * ((Window *) data);
            XDeleteProperty(QX11Info::display(), leader, atoms[ 1 ]);
        }
        XFree(data);
    }
#endif
}

void knotesAuthors(K4AboutData &aboutData)
{
    aboutData.addAuthor(ki18n("Laurent Montel"),
                        ki18n("Maintainer"),
                        "montel@kde.org");
    aboutData.addAuthor(ki18n("Guillermo Antonio Amaral Bastidas"),
                        ki18n("Previous Maintainer"),
                        "me@guillermoamaral.com");
    aboutData.addAuthor(ki18n("Michael Brade"),
                        ki18n("Previous Maintainer"),
                        "brade@kde.org");
    aboutData.addAuthor(ki18n("Bernd Johannes Wuebben"),
                        ki18n("Original KNotes Author"),
                        "wuebben@kde.org");
    aboutData.addAuthor(ki18n("Wynn Wilkes"),
                        ki18n("Ported KNotes to KDE 2"),
                        "wynnw@calderasystems.com");
    aboutData.addAuthor(ki18n("Daniel Martin"),
                        ki18n("Network Interface"),
                        "daniel.martin@pirack.com");
    aboutData.addAuthor(ki18n("Bo Thorsen"),
                        ki18n("Started KDE Resource Framework Integration"),
                        "bo@sonofthor.dk");
    aboutData.addCredit(ki18n("Bera Debajyoti"),
                        ki18n("Idea and initial code for the new look & feel"),
                        "debajyotibera@gmail.com");
    aboutData.addCredit(ki18n("Matthias Ettrich"),
                        KLocalizedString(),
                        "ettrich@kde.org");
    aboutData.addCredit(ki18n("David Faure"),
                        KLocalizedString(),
                        "faure@kde.org");
    aboutData.addCredit(ki18n("Matthias Kiefer"),
                        KLocalizedString(),
                        "kiefer@kde.org");
    aboutData.addCredit(ki18n("Luboš Luňák"),
                        KLocalizedString(),
                        "l.lunak@kde.org");
    aboutData.addCredit(ki18n("Dirk A. Mueller"),
                        KLocalizedString(),
                        "dmuell@gmx.net");
    aboutData.addCredit(ki18n("Carsten Pfeiffer"),
                        KLocalizedString(),
                        "pfeiffer@kde.org");
    aboutData.addCredit(ki18n("Harri Porten"),
                        KLocalizedString(),
                        "porten@kde.org");
    aboutData.addCredit(ki18n("Espen Sand"),
                        KLocalizedString(),
                        "espen@kde.org");
}
