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

#include <AkonadiCore/entitytreemodel.h>
#include <Akonadi/KMime/SpecialMailCollections>
#include <AkonadiCore/selectionproxymodel.h>
#include <messagecomposer/helper/messagefactory.h>

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
class ComposerView;
class ConfigWidget;
class FilterModel;
class KJob;
class KSelectionProxyModel;
class MailThreadGrouperComparator;
class MessageListSettings;
class MessageListSettingsController;
class QStandardItemModel;
class TemplateEmailModel;
class ThreadGrouperModel;
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
               "    <method name=\"openComposerAndAttach\">\n"
               "      <arg direction=\"out\" type=\"i\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"to\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"cc\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"bcc\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"subject\"/>\n"
               "      <arg direction=\"in\" type=\"s\" name=\"body\"/>\n"
               "      <arg direction=\"in\" type=\"as\" name=\"attachments\"/>\n"
               "    </method>\n"
               "  </interface>\n"
                       "")

  Q_PROPERTY( bool collectionIsSentMail READ collectionIsSentMail NOTIFY currentCollectionChanged )

  public:
    explicit MainView(QWidget* parent = 0);

    ~MainView();

    void handleCommandLine();

    enum ForwardMode {
      InLine = 0,
      AsAttachment,
      Redirect
    };

    void setConfigWidget( ConfigWidget *configWidget );

    bool collectionIsSentMail() const;

  public Q_SLOTS:
    void startComposer();
    void restoreDraft( quint64 id );
    void restoreTemplate( quint64 id );

    void markImportant( bool checked );
    void markMailTask( bool checked );
    void modifyDone( KJob *job );
    void dataChanged();

    bool isDraftThreadContent( int row );
    bool isDraftThreadRoot( int row );
    bool isSingleMessage( int row );
    bool folderIsDrafts( const Akonadi::Collection &collection );
    bool isTemplateThreadContent( int row );
    bool isTemplateThreadRoot( int row );
    bool folderIsTemplates( const Akonadi::Collection &collection );

    void configureIdentity();

    int emailTemplateCount();
    void newMessageFromTemplate( int index );

    void selectNextUnreadMessage();

    Q_SCRIPTABLE int openComposer( const QString & to,
                                   const QString & cc,
                                   const QString & bcc,
                                   const QString & subject,
                                   const QString & body );

    Q_SCRIPTABLE int openComposerAndAttach( const QString & to,
                                            const QString & cc,
                                            const QString & bcc,
                                            const QString & subject,
                                            const QString & body,
                                            const QStringList & attachments );
    void mailActionStateUpdated();

  Q_SIGNALS:
    void currentCollectionChanged();

  protected slots:
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
    void preferHtmlViewer( bool useHtml );
    void loadExternalReferences( bool load );
    void folderChanged();
    void moveToOrEmptyTrash();
    void useFixedFont();
    void applyFilters();
    void applyFiltersBulkAction();

    void itemSelectionChanged();
    void slotCollectionSelectionChanged();

  protected:
    void doDelayedInit();
    virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                             QItemSelectionModel *itemSelectionModel );

    virtual void setupAgentActionManager( QItemSelectionModel *selectionModel );

    virtual QAbstractProxyModel* createMainProxyModel() const;
    virtual QAbstractProxyModel* createItemFilterModel() const;

    virtual ImportHandlerBase* importHandler() const;
    virtual ExportHandlerBase* exportHandler() const;

    virtual QAbstractItemModel* createItemModelContext(QDeclarativeContext* context, QAbstractItemModel* model);

    virtual bool doNotUseFilterLineEditInCurrentState() const;

  private slots:
    void qmlInitialized( QDeclarativeView::Status status );
    void sendAgainFetchResult( KJob *job );
    void replyFetchResult( KJob *job );
    void forwardFetchResult( KJob *job );
    void composeFetchResult( KJob *job );
    void initDefaultFolders();
    void createDefaultCollectionDone( KJob *job);
    void deleteItemResult( KJob *job );
    void templateFetchResult( KJob *job );
    void updateConfig();
    bool askToGoOnline();
    void showMessageSource();
    void selectOverrideEncoding();
    void toggleShowExtendedHeaders( bool );
    void messageListSettingsChanged( const MessageListSettings& );
    bool selectNextUnreadMessageInCurrentFolder();
    void showTemplatesHelp();
    void slotDeleteMessage( const Akonadi::Item &item );

  private:
    void reply( quint64 id, MessageComposer::ReplyStrategy replyStrategy, bool quoteOriginal = true );
    void forward( quint64 id, ForwardMode mode );
    void findCreateDefaultCollection( Akonadi::SpecialMailCollections::Type type );
    void recoverAutoSavedMessages();
    Akonadi::Item currentItem() const;
    MessageViewer::MessageViewItem *messageViewerItem();
    uint currentFolderIdentity() const;
    QString itemStorageCollectionAsPath( const Akonadi::Item& ) const;
    KMime::Content *createAttachment( const KUrl &url ) const;

    bool mAskingToGoOnline;
    QWidget *mTransportDialog;
    Akonadi::StandardMailActionManager *mMailActionManager;
    Akonadi::EntityMimeTypeFilterModel *mCollectionModel;
    TemplateEmailModel *mEmailTemplateModel;
    QItemSelectionModel *mTemplateSelectionModel;
    KSelectionProxyModel *m_threadContentsModel;
    MailThreadGrouperComparator *m_grouperComparator;
    ThreadGrouperModel *m_threadGrouperModel;
    ThreadModel *m_threadsModel;
    FilterModel *mFilterModel;
    Akonadi::QuotaColorProxyModel *mQuotaColorProxyModel;
    AclEditor *mAclEditor;
    MessageListSettingsController *mMessageListSettingsController;
    Akonadi::Collection mCurrentCollection;
};

Q_DECLARE_METATYPE( MainView::ForwardMode )

#endif
