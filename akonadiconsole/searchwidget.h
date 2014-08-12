/*
    This file is part of Akonadi.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef AKONADICONSOLE_SEARCHWIDGET_H
#define AKONADICONSOLE_SEARCHWIDGET_H

#include <QWidget>

class KComboBox;
class KJob;
class QTextBrowser;
class KTextEdit;

class QListView;
class QModelIndex;
class QStringListModel;

class SearchWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit SearchWidget( QWidget *parent = 0 );
    ~SearchWidget();

  private Q_SLOTS:
    void search();
    void searchFinished( KJob * );
    void querySelected( int );
    void fetchItem( const QModelIndex & );
    void itemFetched( KJob * );

  private:
    KComboBox *mQueryCombo;
    QTextBrowser *mItemView;
    KTextEdit *mQueryWidget;
    QListView *mResultView;
    QStringListModel *mResultModel;
};

#endif
