/***************************************************************************
                          kimportpage.h  -  description
                             -------------------
    begin                : Fri Jan 17 2003
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KIMPORTPAGE_H
#define KIMPORTPAGE_H

#include "ui_kimportpagedlg.h"
#include <QStyledItemDelegate>
class QTextDocument;

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

class KImportPageDlg : public QWidget, public Ui::KImportPageDlg
{
public:
  KImportPageDlg( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};


class KImportPage : public KImportPageDlg  {
	Q_OBJECT
public:
	KImportPage(QWidget *parent=0);
	~KImportPage();
};

#endif
