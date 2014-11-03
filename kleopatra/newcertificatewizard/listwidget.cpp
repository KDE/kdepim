/* -*- mode: c++; c-basic-offset:4 -*-
    newcertificatewizard/listwidget.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "listwidget.h"

#include "ui_listwidget.h"

#include <QIcon>

#include <QItemSelectionModel>
#include <QStringListModel>
#include <QRegExp>
#include <QRegExpValidator>
#include <QItemDelegate>
#include <QLineEdit>

using namespace Kleo::NewCertificateUi;

namespace
{

class ItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ItemDelegate(QObject *p = 0)
        : QItemDelegate(p), m_rx() {}
    explicit ItemDelegate(const QRegExp &rx, QObject *p = 0)
        : QItemDelegate(p), m_rx(rx) {}

    void setRegExpFilter(const QRegExp &rx)
    {
        m_rx = rx;
    }
    const QRegExp &regExpFilter() const
    {
        return m_rx;
    }

    /* reimp */ QWidget *createEditor(QWidget *p, const QStyleOptionViewItem &o, const QModelIndex &i) const
    {
        QWidget *w = QItemDelegate::createEditor(p, o, i);
        if (!m_rx.isEmpty())
            if (QLineEdit *const le = qobject_cast<QLineEdit *>(w)) {
                le->setValidator(new QRegExpValidator(m_rx, le));
            }
        return w;
    }
private:
    QRegExp m_rx;
};
}

class ListWidget::Private
{
    friend class ::Kleo::NewCertificateUi::ListWidget;
    ListWidget *const q;
public:
    explicit Private(ListWidget *qq)
        : q(qq),
          stringListModel(),
          ui(q)
    {
        ui.listView->setModel(&stringListModel);
        ui.listView->setItemDelegate(&delegate);
        connect(ui.listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                q, SLOT(slotSelectionChanged()));
        connect(&stringListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                q, SIGNAL(itemsChanged()));
        connect(&stringListModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                q, SIGNAL(itemsChanged()));
        connect(&stringListModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                q, SIGNAL(itemsChanged()));
    }

private:
    void slotAdd()
    {
        const int idx = stringListModel.rowCount();
        if (stringListModel.insertRows(idx, 1)) {
            stringListModel.setData(stringListModel.index(idx), defaultValue);
            editRow(idx);
        }
    }

    void slotRemove()
    {
        const int idx = selectedRow();
        stringListModel.removeRows(idx, 1);
        selectRow(idx);
    }

    void slotUp()
    {
        const int idx = selectedRow();
        swapRows(idx - 1, idx);
        selectRow(idx - 1);
    }

    void slotDown()
    {
        const int idx = selectedRow();
        swapRows(idx, idx + 1);
        selectRow(idx + 1);
    }

    void slotSelectionChanged()
    {
        enableDisableActions();
    }

private:
    void editRow(int idx)
    {
        const QModelIndex mi = stringListModel.index(idx);
        if (!mi.isValid()) {
            return;
        }
        ui.listView->setCurrentIndex(mi);
        ui.listView->edit(mi);
    }

    QModelIndexList selectedIndexes() const
    {
        return ui.listView->selectionModel()->selectedRows();
    }
    int selectedRow() const
    {
        const QModelIndexList mil = selectedIndexes();
        return mil.empty() ? -1 : mil.front().row() ;
    }
    void selectRow(int idx)
    {
        const QModelIndex mi = stringListModel.index(idx);
        if (mi.isValid()) {
            ui.listView->selectionModel()->select(mi, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
    }
    void swapRows(int r1, int r2)
    {
        if (r1 < 0 || r2 < 0 || r1 >= stringListModel.rowCount() || r2 >= stringListModel.rowCount()) {
            return;
        }
        const QModelIndex m1 = stringListModel.index(r1);
        const QModelIndex m2 = stringListModel.index(r2);
        const QVariant data1 = m1.data();
        const QVariant data2 = m2.data();
        stringListModel.setData(m1, data2);
        stringListModel.setData(m2, data1);
    }
    void enableDisableActions()
    {
        const QModelIndexList mil = selectedIndexes();
        ui.removeTB->setEnabled(!mil.empty());
        ui.upTB->setEnabled(mil.size() == 1 && mil.front().row() > 0);
        ui.downTB->setEnabled(mil.size() == 1 && mil.back().row() < stringListModel.rowCount() - 1);
    }

private:
    QStringListModel stringListModel;
    ItemDelegate delegate;
    QString defaultValue;
    struct UI : Ui_ListWidget {
        explicit UI(ListWidget *q)
            : Ui_ListWidget()
        {
            setupUi(q);

            addTB->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
            removeTB->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
            upTB->setIcon(QIcon::fromTheme(QLatin1String("go-up")));
            downTB->setIcon(QIcon::fromTheme(QLatin1String("go-down")));
        }
    } ui;

};

ListWidget::ListWidget(QWidget *p)
    : QWidget(p), d(new Private(this))
{

}

ListWidget::~ListWidget() {}

QStringList ListWidget::items() const
{
    return d->stringListModel.stringList();
}

void ListWidget::setItems(const QStringList &items)
{
    d->stringListModel.setStringList(items);
}

QRegExp ListWidget::regExpFilter() const
{
    return d->delegate.regExpFilter();
}

void ListWidget::setRegExpFilter(const QRegExp &rx)
{
    d->delegate.setRegExpFilter(rx);
}

QString ListWidget::defaultValue() const
{
    return d->defaultValue;
}

void ListWidget::setDefaultValue(const QString &df)
{
    d->defaultValue = df;
}

#include "moc_listwidget.cpp"
#include "listwidget.moc"
