/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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
#ifndef KDECLARATIVEMAINVIEW_H
#define KDECLARATIVEMAINVIEW_H

#include "kdeclarativefullscreenview.h"

#include <QItemSelectionModel>

class ExportHandlerBase;
class GuiStateManager;
class ImportHandlerBase;
class KLineEdit;
class ListProxy;
class QAbstractItemModel;
class QAbstractProxyModel;

namespace Akonadi {
class AgentActionManager;
class ChangeRecorder;
class EntityTreeModel;
class Item;
class ItemFetchScope;
}

class KDeclarativeMainViewPrivate;

/**
 * Main view for mobile applications. This class is just to share code and therefore
 * should not be instantiated by itself.
 */
class MOBILEUI_EXPORT KDeclarativeMainView : public KDeclarativeFullScreenView
{
    Q_OBJECT
    Q_PROPERTY( int numSelectedAccounts READ numSelectedAccounts NOTIFY numSelectedAccountsChanged )
    Q_PROPERTY( bool isLoadingSelected READ isLoadingSelected NOTIFY isLoadingSelectedChanged )
    Q_PROPERTY( QString version READ version CONSTANT )
    Q_PROPERTY( QString name READ name CONSTANT )
    Q_PROPERTY( QString state READ applicationState WRITE setApplicationState NOTIFY stateChanged )

  public:
    /**
     * Destroys the declarative main view.
     */
    virtual ~KDeclarativeMainView();

    /**
     * Item fetch scope to specify how much data should be loaded for the list view.
     * By default nothing is loaded.
     */
    Akonadi::ItemFetchScope& itemFetchScope();

    /**
     * Adds a mime type of the items handled by this application.
     */
    void addMimeType( const QString &mimeType );

    /**
     * Returns the mime types of the items handled by this application.
     */
    QStringList mimeTypes() const;

    /**
     * Returns the number of selected accounts.
     */
    int numSelectedAccounts();

    /**
     * Returns the version of the application.
     */
    QString version() const;

    /**
     * Returns the localized name of the application.
     */
    QString name() const;

    /**
     * Returns the monitor that is used by the application.
     */
    Akonadi::ChangeRecorder* monitor() const;

    /**
     * Returns the gui state manager that will be used to manage the visibility
     * of the various gui elements of the application.
     */
    GuiStateManager* guiStateManager() const;

    /**
     * Sets the @p lineEdit that is used to filter the items in the listview.
     */
    void setFilterLineEdit( KLineEdit *lineEdit );

    /**
     * Sets the @p lineEdit that is used to filter the items in the listview in bulk action mode.
     */
    void setBulkActionFilterLineEdit( KLineEdit *lineEdit );

    QString applicationState() const;
    void setApplicationState( const QString &state );

  public slots:
    void setSelectedAccount( int row );

    void setAgentInstanceListSelectedRow( int row );

    /**
     * Starts the account wizard to add and configure new resources.
     */
    void launchAccountWizard();

    /**
     * Starts the synchronization of all collections.
     */
    void synchronizeAllItems();

    /**
     * Opens the licenses.pdf in an external viewer
     */
    void openLicenses();

    void saveFavorite();
    void loadFavorite( const QString &name );
    void multipleSelectionFinished();

    void persistCurrentSelection( const QString &key );
    void clearPersistedSelection( const QString &key );
    void restorePersistedSelection( const QString &key );

    /**
     * Starts the import of items to the application.
     *
     * The actual work is done by the ImportHandlerBase objects returned
     * by the importHandler() method.
     */
    void importItems();

    /**
     * Starts the export of items from the application.
     *
     * The actual work is done by the ExportHandlerBase objects returned
     * by the exportHandler() method.
     */
    void exportItems();

    /**
     * Starts the export of a single item from the application.
     *
     * The actual work is done by the ExportHandlerBase objects returned
     * by the exportHandler() method.
     */
    void exportSingleItem();

    /**
     * Opens the user manual of this application in an external web browser.
     */
    void openManual();

    /**
     * Opens the HTML based documentation located at the given relative @p path.
     */
    void openDocumentation( const QString &path );

    void openAttachment( const QString &url, const QString &mimeType );
    void saveAttachment( const QString &url, const QString &defaultFileName = QString() );

    void reportBug();

    void checkAllBulkActionItems( bool select );

  Q_SIGNALS:
    void numSelectedAccountsChanged();
    void isLoadingSelectedChanged();
    void stateChanged();

    /**
     * This signal is emitted whenever the collection has been changed but
     * before the item list is updated.
     */
    void collectionSelectionChanged();

  protected:
    /**
     * Creates a new main view for a mobile application.
     *
     * @param appName is used to find the QML file in ${DATA_DIR}/mobile/appname.qml
     * @param listProxy proxy for the list view of the application. KDeclarativeMainView
     *                  takes ownwership over the pointer.
     * @param parent The parent widget.
     */
    KDeclarativeMainView( const QString &appName, ListProxy *listProxy, QWidget *parent = 0 );

    /**
     * Returns the global entity tree model.
     */
    Akonadi::EntityTreeModel* entityTreeModel() const;

    /**
     * Returns the filtered and QML-adapted item model.
     */
    QAbstractItemModel* itemModel() const;

    /**
     * Returns whether the currently selected item is being loaded.
     * Note that results appear asynchronously in chunks while loading the contents
     * of a collection. That means that the number of items can be greater then zero
     * while isLoadingSelected returns true.
     */
    bool isLoadingSelected();

    /**
     * Initializes the standard action manager that will be used by the application.
     * This is a point of extension to use a custom action manager.
     *
     * @param collectionSelectionModel The selection model for the collections.
     * @param itemSelectionModel The selection model for the items.
     */
    virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                             QItemSelectionModel *itemSelectionModel );

    /**
     * Initializes the agent action manager that will be used by the application.
     * This is a point of extension to use a custom action manager.
     *
     * @param selectionModel The selection model for the agent instances.
     */
    virtual void setupAgentActionManager( QItemSelectionModel *selectionModel ) = 0;

    /**
     * Returns the a proxy model that will be used on top of the entity tree model.
     */
    virtual QAbstractProxyModel* createMainProxyModel() const;

    /**
     * Returns the filter proxy model that will be used to filter the item list.
     * If @c 0 is returned, no filtering is done.
     *
     * @note The model has to provide a public slot with the following signature:
     *       void setFilterString( const QString& )
     */
    virtual QAbstractProxyModel* createItemFilterModel() const;

    /**
     * Set the filter proxy model that will be used to filter the item list.
     * Call this when calling createItemFilterModel() manually in your code.
     */
    void setItemFilterModel( QAbstractProxyModel* model );

    /**
     * Returns the object that will be used for importing data.
     * If @c 0 is returned, no import functionality is offered.
     */
    virtual ImportHandlerBase* importHandler() const;

    /**
     * Returns the object that will be used for exporting data.
     * If @c 0 is returned, no export functionality is offered.
     */
    virtual ExportHandlerBase* exportHandler() const;

    /**
     * Returns the gui state manager that will be used by the application.
     *
     * Subclasses should returns its custom gui state managers here.
     */
    virtual GuiStateManager* createGuiStateManager() const;

    /**
     * This method is called when a single @p item has been selected to view.
     */
    virtual void viewSingleItem( const Akonadi::Item &item );

    /**
     * Returns whether the application is in a state where the filter line edit
     * can be used.
     */
    virtual bool useFilterLineEditInCurrentState() const;

    /**
     * Returns whether the application is in a state where the filter line edit
     * must not be used.
     */
    virtual bool doNotUseFilterLineEditInCurrentState() const;

  protected Q_SLOTS:
    void breadcrumbsSelectionChanged();
    void itemSelectionChanged();

  protected:
    /**
     * The selection model that belongs to the item model returned by entityTreeModel()
     * or to the one returned by createMainProxyModel().
     */
    QItemSelectionModel* regularSelectionModel() const;

    /**
     * The selection model which provides the information about checked
     * items in the bulk action screen.
     */
    QItemSelectionModel* itemActionModel() const;

    QAbstractProxyModel* itemFilterModel() const;
    QAbstractProxyModel* listProxy() const;
    QItemSelectionModel* itemSelectionModel() const;
    QAbstractItemModel* selectedItemsModel() const;

    Akonadi::Item itemFromId( quint64 id ) const;

    virtual void keyPressEvent( QKeyEvent *event );

    virtual QAbstractItemModel* createItemModelContext( QDeclarativeContext *context, QAbstractItemModel *model );
    void setItemNaigationAndActionSelectionModels( QItemSelectionModel *itemNavigationSelectionModel, QItemSelectionModel *itemActionSelectionModel );

    /**
     * Returns a newly created AgentActionMananger with standard setup.
     * Use inside setupAgentActionManager();
     */
    Akonadi::AgentActionManager *createAgentActionManager( QItemSelectionModel* agentSelectionModel );

  private:
    void doDelayedInitInternal();
    KDeclarativeMainViewPrivate * const d;

    Q_PRIVATE_SLOT( d, void filterLineEditChanged( const QString& ) )
    Q_PRIVATE_SLOT( d, void bulkActionFilterLineEditChanged( const QString& ) )
};

#endif // KDECLARATIVEMAINVIEW_H
