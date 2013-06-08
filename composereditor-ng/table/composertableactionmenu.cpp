/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
#include "helper/tablehelper_p.h"

#include <KLocale>

#include <QWebElement>
#include <QDebug>

namespace ComposerEditorNG
{
class ComposerTableActionMenuPrivate
{
public:
    ComposerTableActionMenuPrivate(QWidget *parent, const QWebElement& element, ComposerTableActionMenu *qq)
        : webElement(element),
          action_insert_table( 0 ),
          action_insert_row_below( 0 ),
          action_insert_row_above( 0 ),
          action_table_format( 0 ),
          action_table_cell_format( 0 ),
          action_remove_cell_contents( 0 ),
          action_remove_cell( 0 ),
          action_insert_cell_before( 0 ),
          action_insert_cell_after( 0 ),
          action_remove_table( 0 ),
          action_remove_row( 0 ),
          action_remove_column( 0 ),
          action_insert_column_before( 0 ),
          action_insert_column_after( 0 ),
          action_merge_cell( 0 ),
          action_split_cell( 0 ),
          q( qq ),
          parentWidget(parent)
    {
    }
    void _k_slotInsertRowBelow();
    void _k_slotInsertRowAbove();
    void _k_slotTableFormat();
    void _k_slotTableCellFormat();
    void _k_slotRemoveCellContents();
    void _k_slotRemoveCell();
    void _k_slotInsertCellBefore();
    void _k_slotInsertCellAfter();
    void _k_slotRemoveTable();
    void _k_slotRemoveRow();
    void _k_slotRemoveColumn();
    void _k_slotInsertColumnBefore();
    void _k_slotInsertColumnAfter();
    void _k_slotMergeCellToTheRight();
    void _k_slotSplitCell();

    void updateActions();
    QWebElement webElement;

    KAction *action_insert_table;
    KAction *action_insert_row_below;
    KAction *action_insert_row_above;
    KAction *action_table_format;
    KAction *action_table_cell_format;
    KAction *action_remove_cell_contents;
    KAction *action_remove_cell;
    KAction *action_insert_cell_before;
    KAction *action_insert_cell_after;
    KAction *action_remove_table;
    KAction *action_remove_row;
    KAction *action_remove_column;
    KAction *action_insert_column_before;
    KAction *action_insert_column_after;
    KAction *action_merge_cell;
    KAction *action_split_cell;
    ComposerTableActionMenu *q;
    QWidget *parentWidget;
};

void ComposerTableActionMenuPrivate::_k_slotSplitCell()
{
    if (webElement.hasAttribute(QLatin1String("colspan"))) {
        webElement.removeAttribute(QLatin1String("colspan"));
    }
    if (webElement.hasAttribute(QLatin1String("rowspan"))) {
        webElement.removeAttribute(QLatin1String("rowspan"));
    }
}

void ComposerTableActionMenuPrivate::_k_slotMergeCellToTheRight()
{
    if (webElement.hasAttribute(QLatin1String("colspan"))) {
        webElement.setAttribute(QLatin1String("colspan"),QString::number(webElement.attribute(QLatin1String("colspan")).toInt() + 1));
    } else {
        webElement.setAttribute(QLatin1String("colspan"),QString::number(2));
    }
}

void ComposerTableActionMenuPrivate::_k_slotInsertColumnAfter()
{
    //TODO
}

void ComposerTableActionMenuPrivate::_k_slotInsertColumnBefore()
{
    //TODO
}

void ComposerTableActionMenuPrivate::_k_slotInsertRowBelow()
{
    QWebElement e = webElement.parent().clone();
    webElement.parent().prependOutside(e);
    TableHelper::removeCellContentsFromCurrentRow(webElement);
}

void ComposerTableActionMenuPrivate::_k_slotInsertRowAbove()
{
    QWebElement e = webElement.parent().clone();
    webElement.parent().appendOutside(e);
    TableHelper::removeCellContentsFromCurrentRow(webElement);
}


void ComposerTableActionMenuPrivate::_k_slotRemoveColumn()
{
    qDebug()<<" tableColumn :"<<TableHelper::tableColumnCount(webElement);
    //TODO
}

void ComposerTableActionMenuPrivate::_k_slotRemoveRow()
{
    if (TableHelper::tableRowCount(webElement) == 1 ) {
        //Remove full table
        QWebElement tableElement = TableHelper::tableWebElement(webElement);
        if (!tableElement.isNull()) {
            tableElement.removeFromDocument();
        }
    } else {
        QWebElement rowElement = TableHelper::rowWebElement(webElement);
        if (!rowElement.isNull()) {
            rowElement.removeFromDocument();
        }
    }
}

void ComposerTableActionMenuPrivate::_k_slotRemoveTable()
{
    QWebElement tableElement = TableHelper::tableWebElement(webElement);
    if (!tableElement.isNull()) {
        tableElement.removeFromDocument();
    }
}

void ComposerTableActionMenuPrivate::_k_slotInsertCellAfter()
{
    QWebElement e = webElement.clone();
    e.setInnerXml(QString());
    webElement.appendOutside(e);
}

void ComposerTableActionMenuPrivate::_k_slotInsertCellBefore()
{
    QWebElement e = webElement.clone();
    e.setInnerXml(QString());
    webElement.prependOutside(e);
}

void ComposerTableActionMenuPrivate::_k_slotRemoveCellContents()
{
    webElement.setInnerXml(QString());
}

void ComposerTableActionMenuPrivate::_k_slotRemoveCell()
{
    webElement.removeFromDocument();
}

void ComposerTableActionMenuPrivate::updateActions()
{
    const bool isACell = (webElement.tagName().toLower() == QLatin1String("td"));
    action_table_cell_format->setEnabled(isACell);
    action_remove_row->setEnabled(isACell);
    action_remove_cell_contents->setEnabled(isACell && !webElement.toInnerXml().isEmpty());
    action_remove_cell->setEnabled(isACell);
    action_insert_cell_after->setEnabled(isACell);
    action_insert_cell_before->setEnabled(isACell);
    action_remove_column->setEnabled(isACell);

    const bool cellIsMerged = (webElement.hasAttribute(QLatin1String("colspan")) || webElement.hasAttribute(QLatin1String("rowspan")) );
    action_split_cell->setEnabled(cellIsMerged);
}


void ComposerTableActionMenuPrivate::_k_slotTableFormat()
{
    ComposerTableFormatDialog dlg( TableHelper::tableWebElement(webElement),parentWidget );
    dlg.exec();
}

void ComposerTableActionMenuPrivate::_k_slotTableCellFormat()
{
    ComposerTableCellFormatDialog dlg( webElement, parentWidget );
    dlg.exec();
}

ComposerTableActionMenu::ComposerTableActionMenu(const QWebElement& element,QObject *parent, QWidget *view)
    : KActionMenu(parent), d(new ComposerTableActionMenuPrivate(view, element, this))
{
    setText( i18n( "Table" ) );

    KActionMenu *insertMenu = new KActionMenu( i18n( "Insert" ), this );
    addAction( insertMenu );

    d->action_insert_table = new KAction( KIcon(QLatin1String("insert-table")), i18nc( "@item:inmenu Insert", "Table..." ), this );
    insertMenu->addAction( d->action_insert_table );
    connect( d->action_insert_table, SIGNAL(triggered(bool)), SIGNAL(insertNewTable()) );

    insertMenu->addSeparator();
    d->action_insert_row_below = new KAction( KIcon(QLatin1String("edit-table-insert-row-below")), i18nc( "@item:inmenu Insert", "Row Below" ), this );
    insertMenu->addAction( d->action_insert_row_below );
    connect( d->action_insert_row_below, SIGNAL(triggered(bool)), SLOT(_k_slotInsertRowBelow()) );

    d->action_insert_row_above = new KAction( KIcon(QLatin1String("edit-table-insert-row-above")), i18nc( "@item:inmenu Insert", "Row Above" ), this );
    insertMenu->addAction( d->action_insert_row_above );
    connect( d->action_insert_row_above, SIGNAL(triggered(bool)), SLOT(_k_slotInsertRowAbove()) );

    insertMenu->addSeparator();
    d->action_insert_column_before = new KAction( KIcon(QLatin1String("edit-table-insert-column-left")), i18nc( "@item:inmenu Insert", "Column Before" ), this );
    insertMenu->addAction( d->action_insert_column_before );
    connect( d->action_insert_column_before, SIGNAL(triggered(bool)), SLOT(_k_slotInsertColumnBefore()) );

    d->action_insert_column_after = new KAction( KIcon(QLatin1String("edit-table-insert-column-right")), i18nc( "@item:inmenu Insert", "Column After" ), this );
    insertMenu->addAction( d->action_insert_column_after );
    connect( d->action_insert_column_after, SIGNAL(triggered(bool)), SLOT(_k_slotInsertColumnAfter()) );

    insertMenu->addSeparator();
    d->action_insert_cell_before = new KAction( i18nc( "@item:inmenu Insert", "Cell Before" ), this );
    insertMenu->addAction( d->action_insert_cell_before );
    connect( d->action_insert_cell_before, SIGNAL(triggered(bool)), SLOT(_k_slotInsertCellBefore()) );

    d->action_insert_cell_after = new KAction( i18nc( "@item:inmenu Insert", "Cell After" ), this );
    insertMenu->addAction( d->action_insert_cell_after );
    connect( d->action_insert_cell_after, SIGNAL(triggered(bool)), SLOT(_k_slotInsertCellAfter()) );

    KActionMenu *removeMenu = new KActionMenu( i18n( "Delete" ), this );
    addAction( removeMenu );

    d->action_remove_table = new KAction( i18nc( "@item:inmenu Delete", "Table" ), this );
    removeMenu->addAction( d->action_remove_table );
    connect( d->action_remove_table, SIGNAL(triggered(bool)), SLOT(_k_slotRemoveTable()) );

    d->action_remove_row = new KAction( i18nc( "@item:inmenu Delete", "Row" ), this );
    removeMenu->addAction( d->action_remove_row );
    connect( d->action_remove_row, SIGNAL(triggered(bool)), SLOT(_k_slotRemoveRow()) );

    d->action_remove_column = new KAction( i18nc( "@item:inmenu Delete", "Column" ), this );
    removeMenu->addAction( d->action_remove_column );
    connect( d->action_remove_column, SIGNAL(triggered(bool)), SLOT(_k_slotRemoveColumn()) );

    d->action_remove_cell = new KAction( i18nc( "@item:inmenu Delete", "Cell" ), this );
    removeMenu->addAction( d->action_remove_cell );
    connect( d->action_remove_cell, SIGNAL(triggered(bool)), SLOT(_k_slotRemoveCell()) );

    d->action_remove_cell_contents = new KAction( i18nc( "@item:inmenu Delete", "Cell Contents" ), this );
    removeMenu->addAction( d->action_remove_cell_contents );
    connect( d->action_remove_cell_contents, SIGNAL(triggered(bool)), SLOT(_k_slotRemoveCellContents()) );

    addSeparator();

    d->action_merge_cell = new KAction( KIcon(QLatin1String("edit-table-cell-merge")), i18n( "Join With Cell to the Right" ), this );
    connect( d->action_merge_cell, SIGNAL(triggered(bool)), SLOT(_k_slotMergeCellToTheRight()) );
    addAction( d->action_merge_cell );

    d->action_split_cell = new KAction( KIcon(QLatin1String("edit-table-cell-split")), i18n( "Split cells" ), this );
    connect( d->action_split_cell, SIGNAL(triggered(bool)), SLOT(_k_slotSplitCell()) );
    addAction( d->action_split_cell );

    addSeparator();

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
