#include "ldapoptionswidgetimpl.h"
#include "addhostdialog.h"
#include <qlistbox.h>
#include <qmessagebox.h>
#include <KDListBoxPair.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qspinbox.h>
#include <klocale.h>

/*
 *  Constructs a LDAPOptionsWidgetImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f' 
 */
LDAPOptionsWidgetImpl::LDAPOptionsWidgetImpl( QWidget* parent,  const char* name, WFlags fl )
  : LDAPOptionsWidget( parent, name, fl )
{
  // a connection to enable or disable Edit and Remove 
  // PushButtons depending on if the left list box is 
  // populated or not
  QObject::connect( listBoxPair->leftListBox(),SIGNAL(selectionChanged()),this, SLOT( checkSelected()));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
LDAPOptionsWidgetImpl::~LDAPOptionsWidgetImpl()
{
  // no need to delete child widgets, Qt does it all for us
}

// the user will not be able to use the Edit and Remove 
// PushButtons if there are no items to edit or remove 
// in the left list box.
void  LDAPOptionsWidgetImpl::checkSelected()
{
  int e = listBoxPair->leftListBox()->currentItem();
  if (e != -1 ) {
    editHostPB->setEnabled(true);
    removeHostPB->setEnabled(true);
  }
  else {
    editHostPB->setEnabled(false);
    removeHostPB->setEnabled(false);
  }

}

/* 
 * This function add a host server in the left list box
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
      
      // Insert the host server adress in the left list box 
      listBoxPair->leftListBox()->insertItem(addDialog.hostEdit->text());

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
  QString keyhost= listBoxPair->leftListBox()->currentText().stripWhiteSpace();

  // the index of the current item in the list box needed to
  // delete the item later.
  int i = listBoxPair->leftListBox()->currentItem();

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
  if (addDialog.exec()&& (addDialog.hostEdit->text() != "")){
    /* In case the user changed some data */

    // first remove the servers'data from the map
    // ( we'll then register the modified data ) 

    for ( it = _ldapservers.begin(); it != _ldapservers.end(); ++it ) {
      if (it.key()== keyhost) {
	_ldapservers.remove(it);
	break;
      }
    }
   
    // remove the item which was edited from the list box
    listBoxPair->leftListBox()->removeItem(i);
    // insert the new data in the list box 
    listBoxPair->leftListBox()->insertItem(addDialog.hostEdit->text());

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
 * left list box and the host server and its associated
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
  QString keyhost= listBoxPair->leftListBox()->currentText().stripWhiteSpace();

  // the index of the current item in the list box needed to
  // delete the item later.
  int i = listBoxPair->leftListBox()->currentItem();


  AddHostDialog addDialog(this, "AddHostDialog", true);
  addDialog.setCaption( i18n("Remove Host") );
  addDialog.show();

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
  if ((addDialog.exec()) && (addDialog.hostEdit->text()!= "")){
    // remove the host server adress from the list box
    listBoxPair->leftListBox()->removeItem(i);

    // Remove the servers'data from the map
    // ( we'll then register the modified data ) 
    for ( it = _ldapservers.begin(); it != _ldapservers.end(); ++it ) {
      if (it.key()== keyhost) {
	_ldapservers.remove(it);
	break;
      }
    }
  }
}


    
  
#include "ldapoptionswidgetimpl.moc"
