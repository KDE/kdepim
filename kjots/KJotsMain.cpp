//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//  Copyright (C) 2007-2008 Stephen Kelly <steveire@gmail.com>
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
#include "kjotscomponent.h"


#include <q3header.h>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QCheckBox>
#include <QTextCharFormat>

//Added by qt3to4:
#include <kactionmenu.h>

#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <kicon.h>
#include <kfinddialog.h>
#include <kfind.h>
#include <kreplacedialog.h>
#include <kreplace.h>
#include <ktemporaryfile.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kbookmarkmenu.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kencodingfiledialog.h>
#include <kglobalsettings.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kfontaction.h>
#include <kfontsizeaction.h>
#include <kstatusbar.h>

#include "KJotsSettings.h"
#include "kjotsbookmarks.h"
#include "kjotsedit.h"
#include "kjotsbrowser.h"
#include "kjotsentry.h"
#include "bookshelf.h"


//----------------------------------------------------------------------
// KJOTSMAIN
//----------------------------------------------------------------------
KJotsMain::KJotsMain()
{

    // Main widget
    //

    KStandardAction::quit(this, SLOT(onQuit()), actionCollection());

    component = new KJotsComponent(this, actionCollection());

    setCentralWidget(component);
    statusBar()->insertItem(QString(), 0, 1);
    statusBar()->setItemAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);

    connect(component, SIGNAL(activeAnchorChanged(const QString &, const QString &)),
            SLOT(activeAnchorChanged(const QString &, const QString &)));

    setupGUI();
    connect(component, SIGNAL(captionChanged(QString)), SLOT(updateCaption(QString)));

}

/*!
    Sets the window caption.
*/
void KJotsMain::updateCaption(QString caption)
{
    setCaption(caption);
}

void KJotsMain::activeAnchorChanged(const QString &anchorTarget, const QString &anchorText)
{
    if (!anchorTarget.isEmpty())
    {
        statusBar()->changeItem(anchorText + " -> " + anchorTarget, 0);
    } else {
        statusBar()->changeItem(QString(), 0);
    }
}

bool KJotsMain::queryClose()
{
    return component->queryClose();
}

void KJotsMain::onQuit()
{
    component->queryClose();
    deleteLater();
    KApplication::kApplication()->quit();
}

