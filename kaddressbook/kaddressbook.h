#ifndef KADDRESSBOOK_H
#define KADDRESSBOOK_H

#include <qwidget.h>
#include <qstring.h>
#include <qdict.h>

#include <kabc/vcardconverter.h>
#include "kaddressbookiface.h"

class ViewManager;
class PrefsDialog;
class LDAPSearchDialogImpl;
class AddresseeEditorDialog;
class IncSearchWidget;

namespace KABC {
  class DistributionListDialog;
  class AddressBook;
  class Addressee;
}

/**
 * This class serves as the main window for KAddressBook.  It handles the
 * menus, toolbars, and status bars as well as creating the view.
 *
 * Most of the actions of the application will be resolved here or in the
 * view manager. Since the application can have multiple views, the actions
 * cannot be connect directly to the views.
 *
 * This class should be used as the main widget of the application. It will
 * hold a VBox, with the view manager on top, and the quick edit widget on
 * the bottom.
 *
 * To interact with the view manager directly, it can be retrieved using
 * viewManager().
 *
 * @short Main window class
 * @author Don Sanders <dsanders@kde.org>
 * @version 0.1
 */
class KAddressBook : public QWidget
{
    Q_OBJECT
  public:
    KAddressBook( QWidget *parent, const char *name=0 );
    virtual ~KAddressBook();

    ViewManager *viewManager() { return mViewManager; }

  public slots:
    /** DCOP METHODS. */
    void addEmail( QString addr );
    void newContact();
    ASYNC showContactEditor( QString uid );
    QString getNameByPhone( QString phone );
    /** END DCOP METHODS */

    /** Displays the distribution list editor dialog.
    *
    * This method name needs to be updated to something better.
    * -mpilone
    */
    void slotDistributionList();

    /** Saves the contents of the AddressBook back to disk.
    */
    void save();

    /** Reads the config file.
    */
    void readConfig();

    /** Writes the config file.
    */
    void writeConfig();

    /** Undo the last command using the undo stack.
    */
    void undo();

    /** Redo the last command that was undone, using the redo stack.
    */
    void redo();

    /** Import libkab data
    */
    void importKDE2();

    /** Import comma-seperated list of all addressbook entries.
    */
    void importCSV();

    /** Import VCard 2.1 files
    */
    void importVCard21();

    /** Import VCard 3.0 files
    */
    void importVCard30();

    /** Import VCard files with given version
    */
    void importVCard( KABC::VCardConverter::Version );

    /** Export comma-seperated list of all addressbook entries.
    */
    void exportCSV();

    /** Export VCard 3.0 files
    */
    void exportVCard30();

    /** Export VCard files with given version
    */
    void exportVCard( KABC::VCardConverter::Version );

    /** Shows the edit dialog for the given uid. If the uid is QString::null,
    *  the method will try to find a selected addressee in the view.
    */
    void editAddressee(QString uid = QString::null);

    /** Creates a new addressee and shows the edit dialog for it. If the
    * new edit dialog is cancel, the addressee will be destroyed. If the
    * edit dialog is accepted, the addressee will be inserted into the view
    * and the view will be told to refresh.
    */
    void newAddressee();

    /** The same like the above method, but a dialog will appear where you
     * can select a resource, to which the addressee should go.
     */
    void newResourceAddressee();

    /**
      Launches the configuration dialog.
    */
    void configure();

    /** Creates a KAddressBookPrinter, which will display the print
    * dialog and do the printing.
    */
    void print();

    /** Displays the Edit Filters dialog box
    */
    void configureFilters();

    /** Make the incremental search widget known. Not elegant, but works.
     */
    void setIncSearchWidget(IncSearchWidget*);

  protected:
    AddresseeEditorDialog *createAddresseeEditorDialog( QWidget *parent,
                                                        const char *name = 0 );

  protected slots:
    /** Called whenever the user selects an entry in the view.
    */
    void addresseeSelected(const QString &uid);

    /** called whenever the user activates an entry in the view.
    */
    void addresseeExecuted(const QString &uid);

    /** Called whenever an addressee is modified. This method will create
    * the proper undo item (new or edit) and update the addressee database.
    */
    void addresseeModified(const KABC::Addressee &);

    /** Called whenever the view is modified in some way. This could
    * mean that the view supports inline editing and the user is
    * editing a contact.
    */
    void viewModified();

    void slotOpenLDAPDialog();

    /** Called whenever the configuration is changed. This happens when
    * the user presses the OK or Apply button in the PrefsDialog.
    */
    void configChanged();

    void slotEditorDestroyed( const QString &uid );

    void slotAddressBookChanged();

  signals:
    /** Emitted whenever an addressee is selected in the view.
    *
    * @param selected True if an addressee was selected, false otherwise.
    */
    void addresseeSelected(bool selected);

    /** Emitted whenever the address book is modified in some way.
    *
    * @param mod True if the address book has been modified, false otherwise.
    */
    void modified(bool mod);

  private:
    /** Given an email address, this method will attempt to parse it into
    * the name and the email address. For example, if <i>rawEmail</i>
    * is "Test Testerson <testerson@kde.org>", <i>fullName</i> will be set
    * to "Test Testerson" and <i>email</i> will be set to "testerson@kde.org".
    */
    void parseEmailAddress(QString rawEmail, QString &fullName, QString &email);

    KABC::AddressBook *mDocument;
    KABC::DistributionListDialog *mDistEditor;
    PrefsDialog *mPrefsDialog;
    LDAPSearchDialogImpl *mLdapSearchDialog;
    ViewManager *mViewManager;
    QDict<AddresseeEditorDialog> mEditorDict;
    IncSearchWidget *mIncSearchWidget;
};

#endif // KADDRESSBOOK_H
