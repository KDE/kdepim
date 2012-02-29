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
#ifndef IMPORTMAILSWIDGET_H
#define IMPORTMAILSWIDGET_H

#include <QWidget>

#include "mailimporter_export.h"
#include <QStyledItemDelegate>

class QTextDocument;
class QListWidgetItem;

namespace Ui {
class ImportMailsWidget;
}
namespace MailImporter {

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


class MAILIMPORTER_EXPORT ImportMailsWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ImportMailsWidget(QWidget *parent = 0);
    ~ImportMailsWidget();

    void setStatusMessage( const QString& status );
    void setFrom( const QString& from );
    void setTo( const QString& to );
    void setCurrent( const QString& current );
    void setCurrent( int percent );
    void setOverall( int percent );
    void addItem( QListWidgetItem* item );
    void setLastCurrentItem();
    void clear();

private:
    Ui::ImportMailsWidget *ui;
};
}

#endif // IMPORTMAILSWIDGET_H
