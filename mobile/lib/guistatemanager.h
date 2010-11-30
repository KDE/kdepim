/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#ifndef GUISTATEMANAGER_H
#define GUISTATEMANAGER_H

#include "mobileui_export.h"

#include <QtCore/QObject>
#include <QtCore/QStack>

/**
 * @short A class that manages the UI states of the mobile PIM applications.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class MOBILEUI_EXPORT GuiStateManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY( bool inHomeScreenState READ inHomeScreenState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inAccountScreenState READ inAccountScreenState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inSingleFolderScreenState READ inSingleFolderScreenState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inMultipleFolderScreenState READ inMultipleFolderScreenState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inBulkActionScreenState READ inBulkActionScreenState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inMultipleFolderSelectionScreenState READ inMultipleFolderSelectionScreenState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inViewSingleItemState READ inViewSingleItemState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inSearchScreenState READ inSearchScreenState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inSearchResultScreenState READ inSearchResultScreenState NOTIFY guiStateChanged )
  Q_PROPERTY( bool inConfigScreenState READ inConfigScreenState NOTIFY guiStateChanged )

  Q_ENUMS( GuiState )

  public:
    /**
     * Describes the state of the visible screens.
     */
    enum GuiState {
      /**
       * The state when the 'Home' entry is selected in the navigation bar.
       */
      HomeScreenState,

      /**
       * The state when an account entry (an Akonadi resource collection) is selected
       * in the navigation bar.
       */
      AccountScreenState,

      /**
       * The state when a folder entry (a normal Akonadi collection) is selected
       * in the navigation bar.
       */
      SingleFolderScreenState,

      /**
       * The state when the 'Home' entry is selected in the navigation bar and
       * the user has selected multiple favorite folders.
       */
      MultipleFolderScreenState,

      /**
       * The state when the user activated the bulk action mode by double clicking on
       * a folder in the navigation bar.
       */
      BulkActionScreenState,

      /**
       * The state when the user activated the multiple folder selection mode by clicking on
       * the 'Select' resp. 'Change Selection' button.
       */
      MultipleFolderSelectionScreenState,

      /**
       * The state when the user selected a single item from a view and the item specific viewer
       * is shown.
       */
      ViewSingleItemState,

      /**
       * The state when the search dialog is shown.
       */
      SearchScreenState,

      /**
       * The state when the user started a search and the results are listed.
       */
      SearchResultScreenState,

      /**
       * The state when the main configuration dialog is shown.
       */
      ConfigScreenState,

      /**
       * Point of extension.
       */
      UserState
    };

    /**
     * Creates a new gui state manager.
     *
     * @param parent The parent object.
     */
    GuiStateManager( QObject *parent = 0 );

    /**
     * Destroys the gui state manager.
     */
    ~GuiStateManager();

  public Q_SLOTS:
    /**
     * Switches from the current gui state to the new gui @p state.
     */
    void switchState( int state );

    /**
     * Switches to the new gui @p state but stores the old one on the internal stack.
     *
     * This method should be used if you want to return to the previous gui state at
     * a later point in the execution.
     */
    void pushState( int state );

    /**
     * Switches to the new gui @p state, if it is not on the top of the stack already,
     * but stores the old one on the internal stack.
     *
     * This method should be used if you want to return to the previous gui state at
     * a later point in the execution.
     */
    void pushUniqueState( int state );

    /**
     * Switches to the gui state previously stored on the internal stack.
     */
    void popState();

    /**
     * Returns the current gui state.
     */
    int currentState() const;

  public:
    /**
     * Returns whether the current state is the home screen state.
     */
    bool inHomeScreenState() const;

    /**
     * Returns whether the current state is the account screen state.
     */
    bool inAccountScreenState() const;

    /**
     * Returns whether the current state is the single folder screen state.
     */
    bool inSingleFolderScreenState() const;

    /**
     * Returns whether the current state is the multiple folder screen state.
     */
    bool inMultipleFolderScreenState() const;

    /**
     * Returns whether the current state is the bulk action screen state.
     */
    bool inBulkActionScreenState() const;

    /**
     * Returns whether the current state is the multiple folder selection screen state.
     */
    bool inMultipleFolderSelectionScreenState() const;

    /**
     * Returns whether the current state is the view single item state.
     */
    bool inViewSingleItemState() const;

    /**
     * Returns whether the current state is the search screen state.
     */
    bool inSearchScreenState() const;

    /**
     * Returns whether the current state is the search result screen state.
     */
    bool inSearchResultScreenState() const;

    /**
     * Returns whether the current state is the config screen state.
     */
    bool inConfigScreenState() const;

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the current state has changed.
     */
    void guiStateChanged();

    /**
     * This signal is emitted whenever the current state has changed.
     *
     * @param oldState The old state the manager was in.
     * @param newState The new state the manager is in.
     */
    void guiStateChanged( int oldState, int newState );

  protected:
    virtual void emitChangedSignal();

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
};

#endif
