/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCEEDITOR_EDITORITEMMANAGER_H
#define INCIDENCEEDITOR_EDITORITEMMANAGER_H

#include "incidenceeditors_ng_export.h"

#include <Akonadi/Calendar/IncidenceChanger>
#include <Collection>
#include <QObject>

namespace Akonadi {
  class Collection;
  class Item;
  class ItemFetchScope;
}

class KJob;

namespace IncidenceEditorNG {

class ItemEditorUi;
class ItemEditorPrivate;

/**
 * Helper class for creating dialogs that let the user create and edit the payload
 * of Akonadi items (e.g. events, contacts, etc). This class supports editting of
 * one item at a time and hanldes all Akonadi specific logic like Item creation,
 * Item modifying and monitoring of changes to the item during editing.
 */
// template <typename PayloadT>
class INCIDENCEEDITORS_NG_EXPORT EditorItemManager : public QObject
{
  Q_OBJECT
  public:
    /**
     * Creates an ItemEditor for a new Item.
     * Receives an option IncidenceChanger, so you can share the undo/redo stack with your
     * application.
     */
    EditorItemManager( ItemEditorUi *ui, Akonadi::IncidenceChanger *changer = 0 );

    /**
     * Destructs the ItemEditor. Unsaved changes will get lost at this point.
     */
    ~EditorItemManager();

    enum ItemState {
      AfterSave,  /**< Returns the last saved item */
      BeforeSave  /**< Returns an item with the original payload before the last save call */
    };

    /**
     * Returns the last saved item with payload or an invalid item when save is
     * not called yet.
     */
    Akonadi::Item item( ItemState state = AfterSave ) const;

    /**
     * Loads the @param item into the editor. The item passed <em>must</em> be
     * a valid item. When the payload is not set it will be fetched.
     */
    void load( const Akonadi::Item &item );

    /**
     * Saves the new or modified item. This method does nothing when the
     * ui is not dirty.
     */
    void save();

    /**
     * Sets the item fetch scope.
     *
     * Controls how much of an item's data is fetched from the server, e.g.
     * whether to fetch the full item payload or only meta data.
     *
     * @param fetchScope The new scope for item fetch operations.
     *
     * @see fetchScope()
     */
    void setFetchScope( const Akonadi::ItemFetchScope &fetchScope );

    /**
     * Returns the item fetch scope.
     *
     * Since this returns a reference it can be used to conveniently modify the
     * current scope in-place, i.e. by calling a method on the returned reference
     * without storing it in a local variable. See the ItemFetchScope documentation
     * for an example.
     *
     * @return a reference to the current item fetch scope
     *
     * @see setFetchScope() for replacing the current item fetch scope
     */
    Akonadi::ItemFetchScope &fetchScope();

    /**
     * Returns the collection where the last item was created.
     * Or an invalid collection if none was created.
     */
    Akonadi::Collection collection() const;

    enum SaveAction {
      Create, /**< A new item was created */
      Modify, /**< An existing item was modified */
      None,   /**< Nothing happened. */
      Move,    /**< An existing item was moved to another collection */
      MoveAndModify /**< An existing item was moved to another collection and modified */
    };

    void setIsCounterProposal( bool isCounterProposal );

  Q_SIGNALS:
    void itemSaveFinished( IncidenceEditorNG::EditorItemManager::SaveAction action );

    void itemSaveFailed( IncidenceEditorNG::EditorItemManager::SaveAction action,
                         const QString &message );

    void revertFinished();
    void revertFailed( const QString &message );

  private:
    ItemEditorPrivate *const d_ptr;
    Q_DECLARE_PRIVATE( ItemEditor )
    Q_DISABLE_COPY( EditorItemManager )

    Q_PRIVATE_SLOT(d_ptr, void itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) )
    Q_PRIVATE_SLOT(d_ptr, void itemFetchResult( KJob* ) )
    Q_PRIVATE_SLOT(d_ptr, void itemMoveResult( KJob* ) )
    Q_PRIVATE_SLOT(d_ptr, void onModifyFinished( int changeId, const Akonadi::Item &item,
                                                 Akonadi::IncidenceChanger::ResultCode resultCode,
                                                 const QString &errorString ) )
    Q_PRIVATE_SLOT(d_ptr, void onCreateFinished( int changeId,
                                                 const Akonadi::Item &item,
                                                 Akonadi::IncidenceChanger::ResultCode resultCode,
                                                 const QString &errorString ) )
    Q_PRIVATE_SLOT(d_ptr, void moveJobFinished( KJob *job ) )
};

class INCIDENCEEDITORS_NG_EXPORT ItemEditorUi
{
  public:
    enum RejectReason {
      ItemFetchFailed,      ///> Either the fetchjob failed or no items where returned
      ItemHasInvalidPayload, ///> The fetched item has an invalid payload
      ItemMoveFailed ///> Item move failed
    };

    virtual ~ItemEditorUi();

    /**
     * Returns whether or not the identifier set contains payload identifiers that
     * are displayed/editable in the Gui.
     */
    virtual bool containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const = 0;

    /**
     * Returns whether or not @param item has a payload type that is supported by
     * the gui.
     */
    virtual bool hasSupportedPayload( const Akonadi::Item &item ) const = 0;

    /**
     * Returns whether or not the values in the ui differ from the original (i.e.
     * either an empty or a loaded item). This method <em>only</em> involves
     * payload fields. I.e. if only the collection in which the item should be
     * stored has changed, this method should return false.
     */
    virtual bool isDirty() const = 0;

    /**
     * Returns whether or not the values in the ui are valid. This method can also
     * be used to update the ui if necessary. The default implementation returns
     * true, so if the ui doesn't need validation there is no need to reimplement
     * this method.
     */
    virtual bool isValid() const;

    /**
     * Fills the ui with the values of the payload of @param item. The item is
     * guaranteed to have a payload.
     */
    virtual void load( const Akonadi::Item &item ) = 0;

    /**
     * Stores the values of the ui into the payload of @param item and returns the
     * item with an updated payload. The returned item must have a valid mimetype
     * too.
     */
    virtual Akonadi::Item save( const Akonadi::Item &item ) = 0;

    /**
     * Returns the currently sellected collection in which the item will be stored.
     */
    virtual Akonadi::Collection selectedCollection() const = 0;

    /**
     * This function is called if for some reason the creation or editting of the
     * item cannot be continued. The implementing class must abort editting at
     * this point.
     */
    virtual void reject( RejectReason reason, const QString &errorMessage = QString() ) = 0;
};

}

#endif
