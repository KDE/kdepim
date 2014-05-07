/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "sievescripttabwidget.h"
#include "sievewidgetpageabstract.h"

#include <QMenu>
#include <KLocalizedString>
#include <KIcon>

#include <QTabBar>

namespace KSieveUi {

SieveScriptTabWidget::SieveScriptTabWidget(QWidget *parent)
    : KTabWidget(parent)
{
    setElideMode( Qt::ElideRight );
    tabBar()->setSelectionBehaviorOnRemove( QTabBar::SelectPreviousTab );
    setDocumentMode(true);
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(slotTabContextMenuRequest(QPoint)) );
}

SieveScriptTabWidget::~SieveScriptTabWidget()
{
}

void SieveScriptTabWidget::slotTabContextMenuRequest( const QPoint &pos )
{
    QTabBar *bar = tabBar();
    const int indexBar = bar->tabAt( bar->mapFrom( this, pos ) );
    QWidget *w = widget(indexBar);
    if (!w)
        return;

    SieveWidgetPageAbstract *page = dynamic_cast<SieveWidgetPageAbstract*>(w);
    if (!page)
        return;
    if ((page->pageType() == SieveWidgetPageAbstract::BlockElsIf) || page->pageType() == SieveWidgetPageAbstract::BlockElse) {
        QMenu menu( this );
        QAction *closeTab = menu.addAction( i18nc( "@action:inmenu", "Close Tab" ) );
        closeTab->setIcon( KIcon( QLatin1String( "tab-close" ) ) );

        QAction *action = menu.exec( mapToGlobal( pos ) );

        if (action == closeTab) {
            Q_EMIT tabCloseRequested(indexBar);
        }
    }
}


}

