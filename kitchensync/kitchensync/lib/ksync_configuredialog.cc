#include <qlayout.h>
#include <qvbox.h>
#include <qpoint.h>
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
  if( !isVisible() ) {
    setup();
  }
  KDialogBase::show();
}

void ConfigureDialog::slotOk() {
  apply( true );
  accept();
  emit ok();
}


void ConfigureDialog::slotCancel() {
  apply( false );
  reject();
}


void ConfigureDialog::addWidget(QWidget* widget, const QString &name, QPixmap* pixmap) {
  QFrame *frame = addPage(name, name, *pixmap);
  widget->reparent(frame, QPoint(0,0));
}

void ConfigureDialog::setup() {
}

void ConfigureDialog::unload() {
}

void ConfigureDialog::apply(bool apply) {

}
