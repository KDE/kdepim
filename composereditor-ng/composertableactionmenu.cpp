/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "composertableactionmenu.h"
#include "composertablecellformatdialog.h"
#include "composertableformatdialog.h"
#include "composereditorutil_p.h"

#include <KLocale>

#include <QWebElement>

namespace ComposerEditorNG
{
class ComposerTableActionMenuPrivate
{
public:
    ComposerTableActionMenuPrivate(QWidget *parent, const QWebElement& element, ComposerTableActionMenu *qq)
        : q( qq ), webElement(element), parentWidget(parent)
    {
    }
    void _k_slotInsertRowBelow();
    void _k_slotTableFormat();
    void _k_slotTableCellFormat();

    void updateActions();
    KAction *action_insert_table;
    KAction *action_insert_row_below;
    KAction *action_table_format;
    KAction *action_table_cell_format;
    ComposerTableActionMenu *q;
    QWebElement webElement;
    QWidget *parentWidget;
};

void ComposerTableActionMenuPrivate::updateActions()
{
    //TODO
}

void ComposerTableActionMenuPrivate::_k_slotInsertRowBelow()
{
    //TODO
}

void ComposerTableActionMenuPrivate::_k_slotTableFormat()
{
    ComposerTableFormatDialog dlg( Util::tableWebElement(webElement),parentWidget );
    dlg.exec();
}

void ComposerTableActionMenuPrivate::_k_slotTableCellFormat()
{
    ComposerTableCellFormatDialog dlg( webElement,parentWidget );
    dlg.exec();
}

ComposerTableActionMenu::ComposerTableActionMenu(const QWebElement& element,QObject *parent, QWidget *view)
    : KActionMenu(parent), d(new ComposerTableActionMenuPrivate(view, element, this))
{
    setText( i18n( "Table" ) );

    KActionMenu *insertMenu = new KActionMenu( i18n( "Insert" ), this );
    addAction( insertMenu );

    d->action_insert_table = new KAction( i18n( "Table..." ), this );
    insertMenu->addAction( d->action_insert_table );
    connect( d->action_insert_table, SIGNAL(triggered(bool)), SIGNAL(insertNewTable()) );

    insertMenu->addSeparator();
    d->action_insert_row_below = new KAction( i18n( "Row Below" ), this );
    insertMenu->addAction( d->action_insert_row_below );
    connect( d->action_insert_row_below, SIGNAL(triggered(bool)), SLOT(_k_slotInsertRowBelow()) );

    d->action_table_format = new KAction( i18n( "Table Format..." ), this );
    connect( d->action_table_format, SIGNAL(triggered(bool)), SLOT(_k_slotTableFormat()) );
    addAction( d->action_table_format );

    d->action_table_cell_format = new KAction( i18n( "Table Cell Format..." ), this );
    connect( d->action_table_cell_format, SIGNAL(triggered(bool)), SLOT(_k_slotTableCellFormat()) );
    addAction( d->action_table_cell_format );

    d->updateActions();
}

ComposerTableActionMenu::~ComposerTableActionMenu()
{
    delete d;
}

}

#include "composertableactionmenu.moc"
