/*
  Copyright (C) 2016 eyeOS S.L.U., a Telefonica company, sales@eyeos.com

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


#include "htmllistaction.h"
#include <KIcon>
#include <KLocalizedString>
#include <KAction>
#include <KActionCollection>

HtmlListAction::HtmlListAction(KActionCollection *ac, QObject *parent)
    : QObject(parent)
{
    initializeActions();
}

HtmlListAction::~HtmlListAction()
{

}

void HtmlListAction::initializeActions()
{
#if 0
    markupAction = new KToggleAction( i18n("Rich Text Editing"), this );
    markupAction->setIcon( KIcon( QLatin1String("switch_html" )) );
    markupAction->setIconText( i18n("Rich Text") );
    markupAction->setToolTip( i18n( "Toggle rich text editing mode" ) );
    actionCollection()->addAction( QLatin1String("html"), markupAction );
    connect( markupAction, SIGNAL(triggered(bool)), SLOT(slotToggleMarkup()) );
#endif
}
