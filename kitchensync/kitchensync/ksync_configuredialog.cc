
#include <config.h>
#include <klocale.h>
#include <kapplication.h>

#include "ksync_configuredialog.h"

using namespace KitchenSync;

 
ConfigureDialog::ConfigureDialog( QWidget *parent, 
				  const char *name,
				  bool modal )
  : KDialogBase( IconList,
		 i18n("Configure KitchenSync"),
		 Ok|Cancel,
		 Ok, parent, name, modal, true ) {
  
  setIconListAllVisible( false );
  resize(400,500);
  
}



ConfigureDialog::~ConfigureDialog() {
}

void ConfigureDialog::show() {
  if( !isVisible() )
    setup();
  KDialogBase::show();
}

void ConfigureDialog::slotOk() {
  apply( true );
  accept();
}


void ConfigureDialog::slotCancel() {
  apply( false );
}



void ConfigureDialog::setup() {
}

void ConfigureDialog::unload() {
}

void ConfigureDialog::apply(bool apply) {

}
