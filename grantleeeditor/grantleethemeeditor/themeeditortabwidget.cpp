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

#include "themeeditortabwidget.h"
#include "editorpage.h"

#include <KLocalizedString>
#include <QMenu>
#include <QIcon>

#include <QTabBar>
using namespace GrantleeThemeEditor;

ThemeEditorTabWidget::ThemeEditorTabWidget(QWidget *parent)
    : QTabWidget(parent)
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

void ThemeEditorTabWidget::slotMainFileNameChanged(const QString &fileName)
{
    QTabBar *bar = tabBar();
    if ( count() < 1 )
        return;
    bar->setTabText(0, i18n("Editor") + QString::fromLatin1(" (%1)").arg(fileName));
}

void ThemeEditorTabWidget::slotTabContextMenuRequest( const QPoint &pos )
{
    if ( count() <= 1 )
        return;

    QTabBar *bar = tabBar();
    const int indexBar = bar->tabAt( bar->mapFrom( this, pos ) );
    QWidget *w = widget(indexBar);
    EditorPage *page = dynamic_cast<EditorPage*>(w);
    if (!page)
        return;

    if (page->pageType() == EditorPage::ExtraPage) {
        QMenu menu( this );
        QAction *closeTab = menu.addAction( i18nc( "@action:inmenu", "Close Tab" ) );
        closeTab->setIcon( QIcon::fromTheme( QLatin1String( "tab-close" ) ) );

        QAction *action = menu.exec( mapToGlobal( pos ) );

        if (action == closeTab) {
            Q_EMIT tabCloseRequested(indexBar);
        }
    }
}


