/*
    This file is part of KAddressbook.
    Copyright (c) 1999 Don Sanders <dsanders@kde.org>

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

#ifndef KADDRESSBOOK_H
#define KADDRESSBOOK_H

#include <qdict.h>
#include <qstring.h>
#include <qwidget.h>

#include <kabc/vcardconverter.h>

#include "kaddressbookiface.h"

class AddresseeEditorDialog;
class IncSearchWidget;
class KCMultiDialog;
class LDAPSearchDialog;
class ViewManager;

namespace KABC {
  class AddressBook;
  class Addressee;
}

/**
  This class serves as the main window for KAddressBook.  It handles the
  menus, toolbars, and status bars as well as creating the view.

  Most of the actions of the application will be resolved here or in the
  view manager. Since the application can have multiple views, the actions
  cannot be connect directly to the views.

  This class should be used as the main widget of the application. It will
  hold a VBox, with the view manager on top, and the quick edit widget on
  the bottom.

  To interact with the view manager directly, it can be retrieved using
  viewManager().

  @short Main window class
  @author Don Sanders <dsanders@kde.org>
  @version 0.1
 */
class KAddressBook : public QWidget
{
  Q_OBJECT

  public:
    KAddressBook( QWidget *parent, const char *name = 0 );
    virtual ~KAddressBook();

    ViewManager *viewManager()const;

  public slots:
    /**
      DCOP METHODS.
     */
    void addEmail( QString addr );
    void newContact();
    ASYNC showContactEditor( QString uid );
    QString getNameByPhone( QString phone );
    /**
      END DCOP METHODS
     */

    /**
      Saves the contents of the AddressBook back to disk.
     */
    void save();

    /**
      Reads the config file.
     */
    void readConfig();

    /**
      Writes the config file.
     */
    void writeConfig();

    /**
      Undo the last command using the undo stack.
     */
    void undo();

    /**
      Redo the last command that was undone, using the redo stack.
     */
    void redo();

    /**
      Import libkab data
     */
    void importKDE2();

    /**
      Import comma-seperated list of all addressbook entries.
     */
    void importCSV();

    /**
      Import VCard files, the version is detected automatically.
     */
    void importVCard( const KURL&, bool );

    /**
      Import VCard file. Simple refers to the fact that no QString is passed
      here, used for menu items
     */
    void importVCardSimple();

    /**
      Export comma-seperated list of all addressbook entries.
     */
    void exportCSV();

    /**
      Export VCard 3.0 files.
     */
    void exportVCard30();

    /**
      Export VCard files with given version.
     */
    void exportVCard( KABC::VCardConverter::Version );

    /**
      Shows the edit dialog for the given uid. If the uid is QString::null,
      the method will try to find a selected addressee in the view.
     */
    void editAddressee( QString uid = QString::null );

    /**
      Creates a new addressee and shows the edit dialog for it. If the
      new edit dialog is cancel, the addressee will be destroyed. If the
      edit dialog is accepted, the addressee will be inserted into the view
      and the view will be told to refresh.
     */
    void newAddressee();

    /**
      Launches the configuration dialog.
    */
    void configure();

    /**
      Creates a KAddressBookPrinter, which will display the print
      dialog and do the printing.
     */
    void print();

    /**
      Displays the Edit Filters dialog box.
     */
    void configureFilters();

  protected:
    AddresseeEditorDialog *createAddresseeEditorDialog( QWidget *parent,
                                                        const char *name = 0 );

  protected slots:
    /**
      Called whenever the user selects an entry in the view.
     */
    void addresseeSelected( const QString &uid );

    /**
      Called whenever the user activates an entry in the view.
     */
    void addresseeExecuted( const QString &uid );

    /**
      Called whenever an addressee is modified. This method will create
      the proper undo item (new or edit) and update the addressee database.
     */
    void addresseeModified( const KABC::Addressee& );

    /**
      Called whenever the view is modified in some way. This could
      mean that the view supports inline editing and the user is
      editing a contact.
     */
    void viewModified();

    void slotOpenLDAPDialog();

    /**
      Called whenever the configuration is changed. This happens when
      the user presses the OK or Apply button in the PrefsDialog.
     */
    void configChanged();

    void slotEditorDestroyed( const QString &uid );

    void slotAddressBookChanged();

  signals:
    /**
      Emitted whenever an addressee is selected in the view.

      @param selected True if an addressee was selected, false otherwise.
     */
    void addresseeSelected( bool selected );

    /**
      Emitted whenever the address book is modified in some way.

      @param mod True if the address book has been modified, false otherwise.
     */
    void modified( bool mod );

  private:
    /**
      Given an email address, this method will attempt to parse it into
      the name and the email address. For example, if <i>rawEmail</i>
      is "Test Testerson <testerson@kde.org>", <i>fullName</i> will be set
      to "Test Testerson" and <i>email</i> will be set to "testerson@kde.org".
     */
    void parseEmailAddress( QString rawEmail, QString &fullName, QString &email );

    KABC::AddressBook *mAddressBook;
    LDAPSearchDialog *mLdapSearchDialog;
    KCMultiDialog *mConfigureDialog;
    ViewManager *mViewManager;
    QDict<AddresseeEditorDialog> mEditorDict;
};

#endif
