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
#include <KToggleAction>

HtmlListAction::HtmlListAction(KActionCollection *ac, QObject *parent)
    : QObject(parent),
      mActionCollection(ac)
{
    initializeActions();
}

HtmlListAction::~HtmlListAction()
{

}

void HtmlListAction::slotListOrderer(bool b)
{
    if (b) {
        mUnorderer->setChecked(false);
        Q_EMIT ordererChanged(HtmlListAction::ListOrderer);
    } else {
        mListOrderer->setChecked(false);
        mUnorderer->setChecked(false);
        Q_EMIT ordererChanged(HtmlListAction::None);
    }
}

void HtmlListAction::slotListUnorderer(bool b)
{
    if (b) {
        mListOrderer->setChecked(false);
        Q_EMIT ordererChanged(HtmlListAction::ListUnorderer);
    } else {
        mListOrderer->setChecked(false);
        mUnorderer->setChecked(false);
        Q_EMIT ordererChanged(HtmlListAction::None);
    }
}

void HtmlListAction::initializeActions()
{
    mListOrderer = new KToggleAction(i18n("List Orderer"), this);
    mActionCollection->addAction( QLatin1String("list_orderer"),  mListOrderer);
    mListOrderer->setIcon( KIcon( QLatin1String("format-list-ordered" )) );
    connect( mListOrderer, SIGNAL(triggered(bool)), SLOT(slotListOrderer(bool)) );

    mUnorderer = new KToggleAction(i18n("List Unorderer"), this);
    mActionCollection->addAction( QLatin1String("list_unorderer"), mUnorderer);
    mUnorderer->setIcon( KIcon( QLatin1String("format-list-unordered" )) );
    connect( mUnorderer, SIGNAL(triggered(bool)), SLOT(slotListUnorderer(bool)) );
}
