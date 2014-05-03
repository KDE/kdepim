/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditortabwidget.h"

#include <KLocalizedString>
#include <KMenu>
#include <KIcon>

#include <QTabBar>
#include <QAction>

SieveEditorTabWidget::SieveEditorTabWidget(QWidget *parent)
    : KTabWidget(parent)
{
    setMovable(true);
    setTabsClosable(true);
    connect(this, SIGNAL(tabCloseRequested(int)), SIGNAL(tabCloseRequestedIndex(int)));
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(slotTabContextMenuRequest(QPoint)) );
}

SieveEditorTabWidget::~SieveEditorTabWidget()
{

}

void SieveEditorTabWidget::slotTabContextMenuRequest(const QPoint &pos)
{
    QTabBar *bar = tabBar();
    if ( count() < 1 )
        return;

    const int indexBar = bar->tabAt( bar->mapFrom( this, pos ) );
    if ( indexBar == -1 )
        return;

    KMenu menu( this );
    QAction *closeTab = menu.addAction( i18nc( "@action:inmenu", "Close Tab" ) );
    closeTab->setIcon( KIcon( QLatin1String( "tab-close" ) ) );

    QAction *allOther = menu.addAction( i18nc("@action:inmenu", "Close All Other Tabs" ) );
    allOther->setEnabled( count() > 1 );
    allOther->setIcon( KIcon( QLatin1String( "tab-close-other" ) ) );

    QAction *action = menu.exec( mapToGlobal( pos ) );

    if ( action == allOther ) { // Close all other tabs
        Q_EMIT tabRemoveAllExclude(indexBar);
    } else if (action == closeTab) {
        Q_EMIT tabCloseRequestedIndex(indexBar);
    }
}
