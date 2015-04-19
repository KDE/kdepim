/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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


#include "findbarlineedit.h"
#include <KCompletion>
#include <QMenu>
#include <QContextMenuEvent>
using namespace MessageViewer;

FindBarLineEdit::FindBarLineEdit(QWidget *parent)
    : KLineEdit(parent)
{

}

FindBarLineEdit::~FindBarLineEdit()
{

}

void FindBarLineEdit::contextMenuEvent( QContextMenuEvent*e )
{
   QMenu *popup = KLineEdit::createStandardContextMenu();
   popup->addSeparator();
   //KF5 add i18n
   popup->addAction( QLatin1String( "Clear History" ), this, SLOT(slotClearHistory()) );
   popup->exec( e->globalPos() );
   delete popup;
}

void FindBarLineEdit::slotClearHistory()
{
    KCompletion *comp = completionObject();
    comp->clear();
}
