/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#ifndef IMPORTLOGWIDGET_H
#define IMPORTLOGWIDGET_H

#include "kdepim_export.h"
#include <QStyledItemDelegate>
#include <QListWidget>

class QTextDocument;

namespace KPIM {
class LogItemDelegate : public QStyledItemDelegate
{
public:
    explicit LogItemDelegate( QObject *parent );
    ~LogItemDelegate();

    virtual QSize sizeHint ( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    virtual void paint ( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    virtual QWidget  *createEditor ( QWidget *, const QStyleOptionViewItem  &, const QModelIndex & ) const;


private:
    QTextDocument* document ( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

class KDEPIM_EXPORT CustomLogWidget : public QListWidget
{
public:
    explicit CustomLogWidget( QWidget * parent = 0 );
    ~CustomLogWidget();

    void addInfoLogEntry( const QString &log );
    void addErrorLogEntry( const QString &log );
    void addTitleLogEntry( const QString &log );

    QString toHtml() const;
    QString toPlainText() const;

private:
    enum ItemType {
        ItemLogType = Qt::UserRole + 1
    };

    enum LogType {
        Title=0,
        Error,
        Info
    };
};
}

#endif /* IMPORTLOGWIDGET_H */

