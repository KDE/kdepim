/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <kfiledialog.h>
#include <kurl.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

void AddressBookConfigBase::slotBrowse()
{
 //   lnePath->setText( KFileDialog::getOpenURL().prettyURL() );
}

void AddressBookConfigBase::slotDefault()
{
    urlReq->setURL( "default" );
}

void AddressBookConfigBase::slotEvolution( bool )
{

}
