/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

#include <akonadi/entitytreemodel.h>
#include <akonadi/kmime/specialmailcollections.h>
#include <akonadi/selectionproxymodel.h>
#include <messagecomposer/messagefactory.h>

namespace Akonadi {
  class EntityMimeTypeFilterModel;
  class QuotaColorProxyModel;
  class StandardMailActionManager;
}

namespace MessageViewer {
  class MessageViewItem;
}

class AclEditor;
class AkonadiSender;
class ConfigWidget;
class FilterModel;
class KJob;
class KSelectionProxyModel;
class QStandardItemModel;
class TemplateEmailModel;
class ThreadModel;

class MainView : public KDeclarativeMainView
{
  Q_OBJECT
  Q_CLASSINFO( "D-Bus Interface", "org.kde.kmailmobile.Composer" )
  Q_CLASSINFO( "D-Bus Introspection", ""
               "  <interface name=\"org.kde.kmailmobile.Composer\">\n"
               "    <method name=\"openComposer\">\n"
               "      <arg direction=\"out\" type=\"i\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"to\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"cc\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"bcc\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"subject\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"body\"/>\n"
               "    </method>\n"
               "  </interface>\n"
                       "")

  public:
    explicit MainView(QWidget* parent = 0);

    ~MainView();

    enum ForwardMode {
      InLine = 0,
      AsAttachment,
      Redirect
    };

    void setConfigWidget( ConfigWidget *configWidget );

  public Q_SLOTS:
    void startComposer();
    void restoreDraft( quint64 id );

    void markImportant( bool checked );
    void markMailTask( bool checked );
    void modifyDone( KJob *job );
    void dataChanged();

    bool isDraft( int row );
    bool isDraftThreadContent( int row );
    bool isDraftThreadRoot( int row );
    bool isSingleMessage( int row );
    bool folderIsDrafts( const Akonadi::Collection &collection );

    // HACK until mark-as-read logic is in messageviewer
    virtual void setListSelectedRow( int row );

    void configureIdentity();

    int emailTemplateCount();
    void newMessageFromTemplate( int index );

    void selectNextUnreadMessage();

    Q_SCRIPTABLE int openComposer( const QString & to,
                                   const QString & cc,
                                   const QString & bcc,
                                   const QString & subject,
                                   const QString & body );
  protected slots:
    void delayedInit();
    void forwardMessage();
    void forwardAsAttachment();
    void redirect();
    void replyToAll();
    void replyToAuthor();
    void replyToMailingList();
    void replyToMessage();
    void replyWithoutQuoting();
    void sendAgain();
    void sendQueued();
    void sendQueuedVia();
    void sendQueuedVia( const QString &transport );
    void saveMessage();
    void findInMessage();
    void preferHTML( bool useHtml );
    void loadExternalReferences( bool load );
    void folderChanged();
    void moveToOrEmptyTrash();
    void createToDo();
    void useFixedFont();
    void applyFilters();

    void itemSelectionChanged();
    void itemActionModelChanged();
    void collectionSelectionChanged();

  protected:
    virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                             QItemSelectionModel *itemSelectionModel );

    virtual void setupAgentActionManager( QItemSelectionModel *selectionModel );

    virtual QAbstractProxyModel* createMainProxyModel() const;
    virtual QAbstractProxyModel* createItemFilterModel() const;

    virtual ImportHandlerBase* importHandler() const;
    virtual ExportHandlerBase* exportHandler() const;
    virtual GuiStateManager* createGuiStateManager() const;

    virtual QAbstractItemModel* createItemModelContext(QDeclarativeContext* context, QAbstractItemModel* model);

  private slots:
    void qmlInitialized( QDeclarativeView::Status status );
    void sendAgainFetchResult( KJob *job );
    void replyFetchResult( KJob *job );
    void forwardFetchResult( KJob *job );
    void composeFetchResult( KJob *job );
    void initDefaultFolders();
    void createDefaultCollectionDone( KJob *job);
    void deleteItemResult( KJob *job );
    void showExpireProperties();
    void templateFetchResult( KJob *job );
    void updateConfig();

  private:
    void reply( quint64 id, MessageComposer::ReplyStrategy replyStrategy, bool quoteOriginal = true );
    void forward( quint64 id, ForwardMode mode );
    void findCreateDefaultCollection( Akonadi::SpecialMailCollections::Type type );
    void recoverAutoSavedMessages();
    Akonadi::Item currentItem() const;
    bool askToGoOnline();
    MessageViewer::MessageViewItem *messageViewerItem();
    bool selectNextUnreadMessageInCurrentFolder();

    bool mAskingToGoOnline;
    QWidget *mTransportDialog;
    Akonadi::StandardMailActionManager *mMailActionManager;
    Akonadi::EntityMimeTypeFilterModel *mCollectionModel;
    TemplateEmailModel *mEmailTemplateModel;
    QItemSelectionModel *mTemplateSelectionModel;
    KSelectionProxyModel *m_threadContentsModel;
    ThreadModel *m_threadsModel;
    FilterModel *mFilterModel;
    Akonadi::QuotaColorProxyModel *mQuotaColorProxyModel;
    AclEditor *mAclEditor;
};

Q_DECLARE_METATYPE( MainView::ForwardMode )

#endif
