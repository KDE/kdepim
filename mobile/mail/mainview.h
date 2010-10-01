/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "kdeclarativemainview.h"

#include <messagecomposer/messagefactory.h>
#include <Akonadi/KMime/SpecialMailCollections>

namespace Akonadi {
  class StandardMailActionManager;
class EntityMimeTypeFilterModel;
}

class AkonadiSender;
class KJob;

/** The new KMMainWidget ;-) */
class MainView : public KDeclarativeMainView
{
  Q_OBJECT
  public:
    explicit MainView(QWidget* parent = 0);

    ~MainView();

    enum ForwardMode {
      InLine = 0,
      AsAttachment,
      Redirect
    };

  public slots:
    void startComposer();
    void restoreDraft( quint64 id );

    void markImportant(bool checked);
    void markMailTask(bool checked);
    void modifyDone(KJob *job);
    void dataChanged();

    bool isDraft( int row );
    bool folderIsDrafts(const Akonadi::Collection & col);

    // HACK until mark-as-read logic is in messageviewer
    virtual void setListSelectedRow(int row);

    void configureIdentity();

    void openAttachment(const QString& url, const QString& mimeType);

  protected slots:
    void delayedInit();
    void forwardMessage();
    void forwardAsAttachment();
    void redirect();
    void replyToAll();
    void replyToAuthor();
    void replyToMailingList();
    void replyToMessage();
    void sendAgain();
    void sendQueued();
    void sendQueuedVia();
    void sendQueuedVia( const QString &transport );
    void saveMessage();
    void findInMessage();
    void preferHTML(bool useHtml);
    void loadExternalReferences(bool load);
    void folderChanged();
    void moveToOrEmptyTrash();
    void createToDo();

  protected:
    virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                             QItemSelectionModel *itemSelectionModel );

    virtual void setupAgentActionManager( QItemSelectionModel *selectionModel );

    virtual QAbstractProxyModel* itemFilterModel() const;

  private slots:
    void sendAgainFetchResult( KJob *job );
    void replyFetchResult( KJob *job );
    void forwardFetchResult( KJob *job );
    void composeFetchResult( KJob *job );
    void initDefaultFolders();
    void createDefaultCollectionDone( KJob *job);
    void deleteItemResult( KJob *job );
    void showExpireProperties();

  private:
    
    void reply( quint64 id, MessageComposer::ReplyStrategy replyStrategy );
    void forward(quint64 id, ForwardMode mode);
    void findCreateDefaultCollection( Akonadi::SpecialMailCollections::Type type );
    void recoverAutoSavedMessages();
    Akonadi::Item currentItem();
    bool askToGoOnline();


    AkonadiSender *mMessageSender;
    bool mAskingToGoOnline;
    QWidget* mTransportDialog;
    Akonadi::StandardMailActionManager* mMailActionManager;
    Akonadi::EntityMimeTypeFilterModel* mCollectionModel;
};

Q_DECLARE_METATYPE(MainView::ForwardMode)

#endif
