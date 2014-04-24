/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2009, 2010 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_FOLDERTREEVIEW_H
#define MAILCOMMON_FOLDERTREEVIEW_H

#include "mailcommon_export.h"
#include "foldertreewidget.h"
#include "util/mailutil.h"

#include <EntityTreeView>
#include <Collection>

class QMouseEvent;

namespace Akonadi {
  class CollectionStatisticsDelegate;
}

namespace MailCommon {

/**
 * This is an enhanced EntityTreeView specially suited for the folders in KMail's
 * main folder widget.
 */
class MAILCOMMON_EXPORT FolderTreeView : public Akonadi::EntityTreeView
{
  Q_OBJECT

  public:
    explicit FolderTreeView( QWidget *parent = 0, bool showUnreadCount = true );

    explicit FolderTreeView( KXMLGUIClient *xmlGuiClient, QWidget *parent = 0,
                             bool showUnreadCount = true );

    virtual ~FolderTreeView();

    void selectNextUnreadFolder( bool confirm = false );
    void selectPrevUnreadFolder( bool confirm = false );

    void showStatisticAnimation( bool anim );

    void disableContextMenuAndExtraColumn();

    void setTooltipsPolicy( FolderTreeWidget::ToolTipDisplayPolicy );

    void restoreHeaderState( const QByteArray &data );

    Akonadi::Collection currentFolder() const;

    void disableSaveConfig();
    void readConfig();

    void updatePalette();

    void keyboardSearch( const QString & ); // reimp
  protected:
    enum Move {
      Next = 0,
      Previous = 1
    };

    void init( bool showUnreadCount );
    void selectModelIndex( const QModelIndex & );
    void setCurrentModelIndex( const QModelIndex & );
    QModelIndex selectNextFolder( const QModelIndex &current );
    bool isUnreadFolder( const QModelIndex &current, QModelIndex &nextIndex,
                         FolderTreeView::Move move, bool confirm );
    void writeConfig();

    void setSortingPolicy( FolderTreeWidget::SortingPolicy policy,
                           bool writeInConfig = false );

    virtual void mousePressEvent( QMouseEvent *e );

  public slots:
    void slotFocusNextFolder();
    void slotFocusPrevFolder();
    void slotSelectFocusFolder();
    void slotFocusFirstFolder();
    void slotFocusLastFolder();

  protected slots:
    void slotHeaderContextMenuRequested( const QPoint & );
    void slotHeaderContextMenuChangeIconSize( bool );
    void slotHeaderContextMenuChangeHeader( bool );
    void slotHeaderContextMenuChangeToolTipDisplayPolicy( bool );
    void slotHeaderContextMenuChangeSortingPolicy( bool );

  signals:
    void changeTooltipsPolicy( FolderTreeWidget::ToolTipDisplayPolicy );
    void manualSortingChanged( bool actif );
    void prefereCreateNewTab( bool );

  private:
    bool ignoreUnreadFolder( const Akonadi::Collection &, bool ) const;
    bool allowedToEnterFolder( const Akonadi::Collection &, bool ) const;
    bool trySelectNextUnreadFolder( const QModelIndex &, MailCommon::Util::SearchDirection, bool );

    FolderTreeWidget::ToolTipDisplayPolicy mToolTipDisplayPolicy;
    FolderTreeWidget::SortingPolicy mSortingPolicy;
    Akonadi::CollectionStatisticsDelegate *mCollectionStatisticsDelegate;
    bool mbDisableContextMenuAndExtraColumn;
    bool mbDisableSaveConfig;
};

}

#endif
