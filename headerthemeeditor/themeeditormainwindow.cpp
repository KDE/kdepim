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

#include "themeeditormainwindow.h"

#include "themeeditorpage.h"

#include <KStandardAction>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KLocalizedString>

ThemeEditorMainWindow::ThemeEditorMainWindow()
    : KXmlGuiWindow()
{
    mThemeEditor = new ThemeEditorPage;
    setCentralWidget(mThemeEditor);
    setupActions();
    setupGUI();
}

ThemeEditorMainWindow::~ThemeEditorMainWindow()
{

}

void ThemeEditorMainWindow::setupActions()
{
    mNewThemeAction = new KAction(i18n("New theme..."),this);
    connect(mNewThemeAction, SIGNAL(triggered(bool)),SLOT(slotNewTheme()));
    actionCollection()->addAction( QLatin1String( "new_theme" ), mNewThemeAction );

    KStandardAction::close( this, SLOT(slotCloseTheme()), actionCollection());
    KStandardAction::quit( kapp, SLOT(quit()), actionCollection() );
}

void ThemeEditorMainWindow::slotCloseTheme()
{
    //TODO
}

void ThemeEditorMainWindow::slotNewTheme()
{
    //TODO
}

#include "themeeditormainwindow.moc"
