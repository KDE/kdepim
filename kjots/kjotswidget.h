/*
    This file is part of KJots.

    Copyright (c) 2008-2009 Stephen Kelly <steveire@gmail.com>

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

#ifndef KJOTSWIDGET_H
#define KJOTSWIDGET_H

#include <QWidget>
#include <QModelIndexList>
#include <akonadi/item.h>

#include <grantlee/templateloader.h>
#include <QItemSelection>

class QTextBrowser;
class QTextCursor;
class QStackedWidget;
class QModelIndex;

class KActionCollection;
class KActionMenu;
class KJob;
class KSelectionProxyModel;
class KTextEdit;
class KXMLGUIClient;

namespace Akonadi
{
class EntityTreeModel;
class Session;
}

class KJotsEdit;
class KJotsTreeView;

class KJotsWidget : public QWidget
{
  Q_OBJECT

public:
  KJotsWidget( QWidget *parent, KXMLGUIClient *xmlGuiclient, Qt::WindowFlags f = 0 );
  ~KJotsWidget();

public slots:
  void prevPage();
  void nextPage();
  void prevBook();
  void nextBook();
  bool canGoNextPage() const;
  bool canGoPreviousPage() const;
  bool canGoNextBook() const;
  bool canGoPreviousBook() const;

  void newPage();
  void newBook();

signals:
  void canGoNextPageChanged( bool );
  void canGoPreviousPageChanged( bool );
  void canGoNextBookChanged( bool );
  void canGoPreviousBookChanged( bool );

protected:
  QString renderSelectionToHtml();
  QString getThemeFromUser();

  void selectNext( int role, int step );

protected slots:
  void renderSelection();
  void changeTheme();
  void exportSelection();

  void deletePage();
  void deleteBook();
  void deleteMultiple();

private slots:
  void delayedInitialization();
  void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
  bool canGo( int role, int step ) const;

  void newPageResult( KJob *job );
  void newBookResult( KJob *job );

  void copySelectionToTitle();
  void configure();

private:
  KXMLGUIClient  *m_xmlGuiClient;
  KJotsEdit      *editor;
  QTextBrowser   *browser;
  QStackedWidget *stackedWidget;
  KActionMenu    *bookmarkMenu;
  Akonadi::EntityTreeModel *m_kjotsModel;
  KSelectionProxyModel *selProxy;
  Grantlee::FileSystemTemplateLoader::Ptr m_loader;
  KJotsTreeView *treeview;
  Akonadi::Session *m_session;
};

#endif
