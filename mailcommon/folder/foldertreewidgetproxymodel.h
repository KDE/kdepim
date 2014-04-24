/*
  Copyright (c) 2009, 2010, 2011 Laurent Montel <montel@kde.org>

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

#ifndef MAILCOMMON_FOLDERTREEWIDGETPROXYMODEL_H
#define MAILCOMMON_FOLDERTREEWIDGETPROXYMODEL_H

#include <Collection>
#include <EntityRightsFilterModel>

namespace MailCommon {

class FolderTreeWidgetProxyModel : public Akonadi::EntityRightsFilterModel
{
  Q_OBJECT

  public:
    enum FolderTreeWidgetProxyModelOption {
      None = 0,
      HideVirtualFolder = 1,
      HideSpecificFolder = 2,
      HideOutboxFolder = 4
    };
    Q_DECLARE_FLAGS( FolderTreeWidgetProxyModelOptions, FolderTreeWidgetProxyModelOption )

    explicit FolderTreeWidgetProxyModel(
      QObject *parent = 0,
      FolderTreeWidgetProxyModelOptions = FolderTreeWidgetProxyModel::None );

    virtual ~FolderTreeWidgetProxyModel();

    virtual Qt::ItemFlags flags ( const QModelIndex &index ) const;

    virtual QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    void setEnabledCheck( bool enable );
    bool enabledCheck() const;

    void setHideVirtualFolder( bool exclude );
    bool hideVirtualFolder() const;

    void setHideSpecificFolder( bool hide );
    bool hideSpecificFolder() const;

    void setHideOutboxFolder( bool hide );
    bool hideOutboxFolder() const;
    void setFilterFolder( const QString &filter );

    void addContentMimeTypeInclusionFilter( const QString &mimeTypes );

    void updatePalette();
    void readConfig();

  protected:
    virtual bool acceptRow( int sourceRow, const QModelIndex &sourceParent ) const;

  private:
    class Private;
    Private *const d;
};

}

#endif
