#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qwidget.h>
#include <qdict.h>
#include <qstringlist.h>

#include <kabc/field.h>

#include "filter.h"

class QComboBox;
class QLineEdit;
class QWidgetStack;
class QResizeEvent;
class QDropEvent;
class QSplitter;
class QTabWidget;
class KConfig;
namespace KABC { class AddressBook; }

class ViewWrapper;
class ViewContainer;
class KAddressBookView;
class JumpButtonBar;
class AddresseeEditorWidget;

/** The view manager manages the views and everything related to them. The
* manager will load the views at startup and display a view when told to
* make one active.
*
* The view manager will also create and manage all dialogs directly related to
* views (ie: AddView, ConfigureView, DeleteView, etc).
*/
class ViewManager : public QWidget
{
  Q_OBJECT

  public:
    ViewManager(KABC::AddressBook *doc, KConfig *config,
                QWidget *parent = 0, const char *name = 0);
    ~ViewManager();

    /** Sets the given view active. This usually means raising the
    * view to the top of the widget stack and telling it
    * to refresh.
    */
    void setActiveView(const QString &name);

   /** Destroys all the currently loaded views. It is important that
    * after calling this method you call setActiveView before
    * the user has a chance to interact with the gui, since no
    * views will be loaded.
    */
    void unloadViews();

    /** Returns a list of all the defined views. This list is guaranteed
    * to always contain at least one view. This list is the 'defined' views,
    * not necessarily the loaded views. However the view will be loaded if
    * it becomes active.
    */
    const QStringList &viewNames() { return mViewNameList; }

    /* Return a list of all the uids of selected contacts.
    */
    QStringList selectedUids();

    /** Used to enable or disable the jump button bar.
    *
    * @param visible True for the widget to be visible, false otherwise
    */
    void setJumpButtonBarVisible(bool visible);

    /** Used to enable or disable the features tab widget.
    *
    * @param visible True for the widget to be visible, false otherwise
    */
    void setFeaturesVisible(bool visible);

    /** Used to enable or disable the details widget.
     */
    void setDetailsVisible(bool visible);

    /**
      Return if the quick edit currently is shown or not.
    */
    bool isQuickEditVisible();

    /** @return The list of filters defined for the application.
    */
    const Filter::List &filters() const { return mFilterList; }

  public slots:
    /** Reads the config file.
    */
    void readConfig();

    /** Writes the config file.
    */
    void writeConfig();

    /** Sends an email to all the selected addressees. This is done by
    * asking the view for a string of "To:'s" and then asking
    * KDE to open the mailer with the information.
    */
    virtual void sendMail();

    /** This slot will delete all the selected entries. This method should
    * be called just 'delete' to be consistant with the other edit methods,
    * but good 'ol C++ wouldn't like that -mpilone
    */
    void deleteAddressee();

    /** Paste a contact into the addressbook from the clipboard.
    */
    void paste();

    /** Copy a contact from the view into the clipboard. This method will
    * copy all selected contacts into the clipboard at once.
    */
    void copy();

    /** Cut a contact from the view into the clipboard. This method will
    * cut all selected contacts into the clpboard at once.
    */
    void cut();

    /** Selects the given addressee or all addressees if uid == QString::null
    */
    void setSelected(QString uid = QString::null, bool selected = true);

   /** Refreshes the active view.
   *
   * @param uid Only refresh the selected uid. If it is QStrign::null, the
   * entire view will be refreshed.
    */
    void refresh(QString uid = QString::null);

    /** Launches the ConfigureView dialog for the active view.
    */
    void modifyView();

    /** Deletes the current view and makes another view active.
    */
    void deleteView();

    /** Displays the add view dialog. If the user confirms, the view
    * will be added.
    */
    void addView();

    /** Updates the filter list to <i>list</i>
    */
    void filtersChanged(const Filter::List &list);

    /** Called whenever a filter is activated
    */
    void filterActivated(int index);

  protected slots:
    /** Handle events on the incremental search widget. */
    void incSearch(const QString& text, int field);

    /** Called whenever the user selects a button from the jump bar
    */
    void jumpToLetter(const QChar &ch);

    /** Called whenever the user drops something in the active view.
    * This method will try to decode what was dropped, and if it was
    * a valid addressee, add it to the addressbook.
    */
    void dropped(QDropEvent *e);

    /** Called whenever the user attempts to start a drag in the view.
    * This method will convert all the selected addressees into text (vcard)
    * and create a drag object.
    */
    void startDrag();

    /** Called whenever an addressee is selected in the view. This method
    * should update the quick edit. The selected() signal will already
    * be emitted, so it does not have to be re-emitted from this method
    */
    void addresseeSelected(const QString &uid);

    /** Called whenever the quick view is modified. This method will emit
    * the modified signal and then tell the view to refresh.
    */
    void quickEditModified();

  signals:
    /** Called whenever the user selects an entry in the view.
    */
    void selected(const QString &uid);

    /** called whenever the user activates an entry in the view.
    */
    void executed(const QString &uid);

    /** Emitted whenever the address book is modified in some way.
    */
    void modified();

    /** Emitted whenever the view configuration changes. This can happen
    * if a user adds a new view or removes a view.
    *
    * @param newActive This is the view that should be made active. If this
    * is QString::null, than the current active can remain that way.
    */
    void viewConfigChanged(const QString &newActive);

    /** Update the IncSearchWidget (in the toolbars) with a new list
        of fields. */
    void setIncSearchFields(const QStringList&);

    /** Update the select_filter action. */
    void setFilterNames(const QStringList&);

    /** Set the current filter by its name. Ignored if the name does not exist. */
    void setCurrentFilterName(const QString&);

    /** Set the current filter. 0 for none. */
    void setCurrentFilter(int index);

    /** Import a VCard that has been dragged  */
    void importVCard(const QString &, bool);

  private:
    /** Create a view wrapper for each type of view we know about
    */
    void createViewWrappers();

    /** Creates the GUI components
    */
    void initGUI();

   /** Populates the incremental search combo box.
    */
    void refreshIncrementalSearchCombo();

    QStringList mViewNameList;
    QDict<ViewWrapper> mViewWrapperDict;
    QDict<KAddressBookView> mViewDict;
    KABC::AddressBook *mDocument;
    KConfig *mConfig;
    QWidgetStack *mViewWidgetStack;
    KABC::Field::List mIncrementalSearchFields;
    KABC::Field *mCurrentIncSearchField;
    Filter::List mFilterList;
    Filter mCurrentFilter;
    // ----- GUI widgets:
    KAddressBookView *mActiveView;
    ViewContainer *mDetails;
    JumpButtonBar *mJumpButtonBar;
    AddresseeEditorWidget *mQuickEdit;
    QSplitter *mQSpltDetails;
    QSplitter *mQSpltFeatures;
    QTabWidget *mFeatures;
};

#endif
