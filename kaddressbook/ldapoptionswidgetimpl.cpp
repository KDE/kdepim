#include <qlistbox.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qspinbox.h>
#include <qheader.h>

#include <klistview.h>
#include <klocale.h>

#include "ldapoptionswidgetimpl.h"
#include "addhostdialog.h"

/*
 *  Constructs a LDAPOptionsWidgetImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
LDAPOptionsWidgetImpl::LDAPOptionsWidgetImpl( QWidget* parent,  const char* name, WFlags fl )
  : LDAPOptionsWidget( parent, name, fl )
{
  // a connection to enable or disable Edit and Remove PushButtons
  QObject::connect( ldapServersLV,SIGNAL(selectionChanged()),this, SLOT( checkSelected()));
  ldapServersLV->addColumn(QString::null);
  ldapServersLV->header()->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
LDAPOptionsWidgetImpl::~LDAPOptionsWidgetImpl()
{
  // no need to delete child widgets, Qt does it all for us
}

// the user will not be able to use the Edit and Remove
// PushButtons if there are no items to edit
void  LDAPOptionsWidgetImpl::checkSelected()
{
  if (ldapServersLV->currentItem() != 0L ) {
    editHostPB->setEnabled(true);
    removeHostPB->setEnabled(true);
  }
  else {
    editHostPB->setEnabled(false);
    removeHostPB->setEnabled(false);
  }

}

/*
 * This function add a host server in the list view
 * and register this host server its associated data in
 * a map in order for us to be able to retrieve this data
 * when needed.
 */
void LDAPOptionsWidgetImpl::addHost()
{
  // create and show thew dialog
  AddHostDialog addDialog(this, "AddHostDialog", true);
  addDialog.show();

  // the user clicked on the "OK" PushButton. And some
  // text has been written.
  if ( addDialog.exec() && !addDialog.hostEdit->text().stripWhiteSpace().isEmpty())
    {

      // Insert the host server adress in the list view
      (void) new QCheckListItem( ldapServersLV, addDialog.hostEdit->text(), QCheckListItem::CheckBox );

      // we have now an adress: enable the edit and remove
      // store the host server and its data
      QString host = addDialog.hostEdit->text().stripWhiteSpace();
      int port = addDialog.portSpinBox->value();
      QString base = addDialog.baseEdit->text().stripWhiteSpace();
      // create a host account
      server.setbase(base);
      server.setport(port);
      // register the host and its data in the map
      _ldapservers[host] = server;
    }
}

/*
 * This function edit the host server selected by the user
 * and the data associated to this host server in a dialog.
 * The host server and its data are retrieved from a map and
 * can be modified by the user.
 */
void LDAPOptionsWidgetImpl::editHost()
{

  // the variables to store the data we will retrieve in
  // the map ( where servers adresses and data are stored
  QString host, base;
  int port = 0;

  // the key to retrieve the data associated to the server
  // ( port and baseDn ) in our map.
  QCheckListItem *item = static_cast<QCheckListItem *>(ldapServersLV->currentItem());
  QString keyhost = item->text(0).stripWhiteSpace();

  // our message box editing the data the user want to
  // consult
  AddHostDialog addDialog(this, "AddHostDialog", true);
  addDialog.setCaption( i18n("Edit Host") );
  addDialog.show();

  // An iterator to loop in the map and find the server and
  // its data
  ServersMap::Iterator it;
  for ( it = _ldapservers.begin(); it != _ldapservers.end(); ++it ) {
    if (it.key()== keyhost) {
      host = it.key();
      port = it.data().port();
      base = it.data().baseDN();
      break;
    }
  }

  // the servers'data was retrieved and populate the
  // the line edit in the message box
  addDialog.hostEdit->setText(host);
  addDialog.portSpinBox->setValue(port);
  addDialog.baseEdit->setText(base);

  // the user clicked on the 'OK' PushButton
  if (addDialog.exec()&& (!addDialog.hostEdit->text().isEmpty())){
    /* In case the user changed some data */

    // first remove the servers'data from the map
    // ( we'll then register the modified data )

    for ( it = _ldapservers.begin(); it != _ldapservers.end(); ++it ) {
      if (it.key()== keyhost) {
	_ldapservers.remove(it);
	break;
      }
    }

    // change text on the current item
    item->setText(0, addDialog.hostEdit->text());

    /* the data has to be registered in the map also */

    // store the new values
    QString ht = addDialog.hostEdit->text().stripWhiteSpace();
    int pt = addDialog.portSpinBox->value();
    QString be = addDialog.baseEdit->text().stripWhiteSpace();

    // open a new server account
    server.setbase(be);
    server.setport(pt);

    // register a new key( with its data ) in the map
    _ldapservers[ht] = server;
  }
}

/*
 * This function remove the host server adress from the
 * list view and the host server and its associated
 * data ( port and baseDN ) from the servers map.
 */
void LDAPOptionsWidgetImpl::removeHost()
{

  // the variables to store the data we will retrieve in
  // the map ( where servers adresses and data are stored
  QString host, base;
  int port = 0;

  // the key to retrieve the data associated to the server
  // ( port and baseDn ) in our map.
  QCheckListItem *item = static_cast<QCheckListItem *>(ldapServersLV->currentItem());
  QString keyhost = item->text(0).stripWhiteSpace();

  ServersMap::Iterator it;
  for ( it = _ldapservers.begin(); it != _ldapservers.end(); ++it ) {
    if (it.key()== keyhost) {
      host = it.key();
      port = it.data().port();
      base = it.data().baseDN();
      break;
    }
  }

  // remove the host server adress from the list box
  delete item;

  // Remove the servers'data from the map
  // ( we'll then register the modified data )
  for ( it = _ldapservers.begin(); it != _ldapservers.end(); ++it ) {
    if (it.key()== keyhost) {
      _ldapservers.remove(it);
      break;
    }
  }
}

#include "ldapoptionswidgetimpl.moc"
