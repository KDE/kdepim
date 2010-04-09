/*
    This file is part of KJots.

    Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
    Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
    Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
    Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
    Copyright (C) 2007-2009 Stephen Kelly <steveire@gmail.com>

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
#include <QItemSelection>

#include <akonadi/item.h>

#include <grantlee/templateloader.h>

class QCheckBox;
class QTextCursor;
class QTextEdit;
class QStackedWidget;
class QModelIndex;

class KActionCollection;
class KActionMenu;
class KFindDialog;
class KJob;
class KReplaceDialog;
class KSelectionProxyModel;
class KTextEdit;
class KTextBrowser;
class KXMLGUIClient;

namespace Akonadi
{
class EntityTreeModel;
class Session;
}

namespace Grantlee
{
class Engine;
}

class KJotsEdit;
class KJotsTreeView;

class KJotsWidget : public QWidget
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.KJotsWidget")

public:
  KJotsWidget( QWidget *parent, KXMLGUIClient *xmlGuiclient, Qt::WindowFlags f = 0 );
  ~KJotsWidget();

  QTextEdit* activeEditor();

public slots:
  void prevPage();
  void nextPage();
  void prevBook();
  void nextBook();
  bool canGoNextPage() const;
  bool canGoPreviousPage() const;
  bool canGoNextBook() const;
  bool canGoPreviousBook() const;

  void updateCaption();
  void updateMenu();

  Q_SCRIPTABLE void newPage();
  Q_SCRIPTABLE void newBook();

signals:
  void canGoNextPageChanged( bool );
  void canGoPreviousPageChanged( bool );
  void canGoNextBookChanged( bool );
  void canGoPreviousBookChanged( bool );

  void captionChanged( const QString &newCaption );

protected:
  QString renderSelectionToHtml();
  QString renderSelectionToXml();
  QString renderSelectionToPlainText();
  QString getThemeFromUser();

  void selectNext( int role, int step );
  int search( bool );
  void migrateNoteData( const QString &migrator, const QString &type = QString() );

protected slots:
  void renderSelection();
  void changeTheme();
  void exportSelectionToHtml();
  void exportSelectionToPlainText();
  void exportSelectionToXml();
  void printSelection();

  void deletePage();
  void deleteBook();
  void deleteMultiple();

private slots:
  void delayedInitialization();
  void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight );

  bool canGo( int role, int step ) const;

  void newPageResult( KJob *job );
  void newBookResult( KJob *job );

  void copySelectionToTitle();
  void copy();
  void configure();

  void onShowSearch();
  void onUpdateSearch();
  void onStartSearch();
  void onRepeatSearch();
  void onEndSearch();

  void onShowReplace();
  void onUpdateReplace();
  void onStartReplace();
  void onRepeatReplace();
  void onEndReplace();


private:
  KXMLGUIClient  *m_xmlGuiClient;
  KJotsEdit      *editor;
  KTextBrowser   *browser;
  QStackedWidget *stackedWidget;
  KActionMenu    *bookmarkMenu;
  Akonadi::EntityTreeModel *m_kjotsModel;
  KSelectionProxyModel *selProxy;
  KJotsTreeView *treeview;
  Akonadi::Session *m_session;

  Grantlee::Engine *m_templateEngine;
  Grantlee::FileSystemTemplateLoader::Ptr m_loader;

  KFindDialog *searchDialog;
  QStringList searchHistory;
  int searchBeginPos, searchEndPos, searchPos;
  QCheckBox *searchAllPages;

  KReplaceDialog *replaceDialog;
  QStringList replaceHistory;
  int replaceBeginPos, replaceEndPos, replacePos;
  QCheckBox *replaceAllPages;
  QModelIndex replaceStartPage;

  QSet<QAction*> entryActions, pageActions, bookActions, multiselectionActions;
};

#endif
