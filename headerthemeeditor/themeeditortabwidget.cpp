/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "themeeditortabwidget.h"

#include <KLocale>
#include <KMenu>
#include <KIcon>

#include <QTabBar>

ThemeEditorTabWidget::ThemeEditorTabWidget(QWidget *parent)
    : KTabWidget(parent)
{
    setElideMode( Qt::ElideRight );
    tabBar()->setSelectionBehaviorOnRemove( QTabBar::SelectPreviousTab );
    setDocumentMode(true);
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(slotTabContextMenuRequest(QPoint)) );
}

ThemeEditorTabWidget::~ThemeEditorTabWidget()
{
}

void ThemeEditorTabWidget::slotTabContextMenuRequest( const QPoint &pos )
{
    QTabBar *bar = tabBar();
    if ( count() <= 1 )
        return;

    const int indexBar = bar->tabAt( bar->mapFrom( this, pos ) );
    if ( indexBar <= 1 )
        return;

    KMenu menu( this );
    QAction *closeTab = menu.addAction( i18nc( "@action:inmenu", "Close Tab" ) );
    closeTab->setIcon( KIcon( QLatin1String( "tab-close" ) ) );

    QAction *action = menu.exec( mapToGlobal( pos ) );

    if (action == closeTab) {
        Q_EMIT tabCloseRequested(indexBar);
    }
}


#include "themeeditortabwidget.moc"
