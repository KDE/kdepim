#include <qpopupmenu.h>
#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include "AddressBookWidget.h"

AddressBookDialog::AddressBookDialog(ContactEntry * e, const char * name = 0)
    :   KTMainWindow(name)
{   
    // Menubar.
 
    QPopupMenu * fileMenu_ = new QPopupMenu;
    fileMenu_->insertItem(i18n("E&xit"), kapp, SLOT(quit()));

    menuBar()->insertItem(i18n("&File"), fileMenu_);
 
    // Toolbar.

  	toolBar()->insertButton(BarIcon("exit.xpm"), 0, SIGNAL(clicked()),
			kapp, SLOT(quit()), true, i18n("E&xit"));
    
    // Main widget.

    mainWidget_ = new QWidget(this, "mainWidget");
    
    // Contact widget.

    layout_     = new QGridLayout(mainWidget_, 2, 1, 10, 10);
    abWidget_   = new ContactDialog(mainWidget_, "abWidget", e);
    
    // Button box.

    buttonBox_  = new KButtonBox(mainWidget_);
    
    pb_help_        = buttonBox_->addButton(i18n("&Help"));
    pb_defaults_    = buttonBox_->addButton(i18n("&Defaults"));
    buttonBox_->addStretch();
    pb_OK_          = buttonBox_->addButton(i18n("&OK"));
    pb_OK_->setDefault(true);
    pb_OK_->setAutoDefault(true);
    pb_apply_       = buttonBox_->addButton(i18n("&Apply"));
    pb_cancel_      = buttonBox_->addButton(i18n("&Cancel"));
    
    buttonBox_->setFixedHeight(buttonBox_->sizeHint().height());
    
    layout_->addWidget(abWidget_,   0, 0);
    layout_->addWidget(buttonBox_,  1, 0);
    
    layout_->activate();
    mainWidget_->setMinimumSize(mainWidget_->minimumSizeHint());

    // Done. Set the view and show yourself.    
    setView(mainWidget_);

    // oh Dear, setView takes a widget not a layout. 
    // And there is no setViewLayout.
    // widget( layout ( widget (layout layout ) )) -> bad sizeHint
    // widget( layout (layout (layout ... ))) -> good sizeHint
    // Hence I need to do the sizing manually! :-(
    // - Don Sanders
    QSize rightSize = QSize( mainWidget_->sizeHint().width(), 
			     mainWidget_->sizeHint().height() + 
			     fileMenu_->sizeHint().height() + 
			     toolBar()->sizeHint().height() );
    resize( rightSize );
    show();
}

AddressBookDialog::~AddressBookDialog()
{
    // empty
}

