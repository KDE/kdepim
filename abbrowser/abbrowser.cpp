/*
 * pab.cpp
 *
 * Copyright (C) 1999 Don Sanders <dsanders@kde.org>
 */
#include "abbrowser.h"
#include "browserentryeditor.h"

#include <qkeycode.h>

//#include <kfm.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmenubar.h>
#include <kconfig.h>

#include "undo.h"
#include "browserwidget.h"
#include "entry.h"

Pab::Pab()
{
  setCaption( i18n( "Address Book Browser" ));
  //xxx  document = new ContactEntryList( "entries.txt" );
  document = new ContactEntryList();
  view = new PabWidget( document, this, "Kontact" );

  // tell the KTMainWindow that this is indeed the main widget
  setView(view);

  // create a popup menu -- in this case, the File menu
  QPopupMenu* p = new QPopupMenu;
  p->insertItem(i18n("&Save"), this, SLOT(save()), CTRL+Key_S);
  p->insertItem(i18n("&New Contact"), this, SLOT(newContact()), CTRL+Key_N);
  p->insertItem(i18n("New &Group"), this, SLOT(newGroup()), CTRL+Key_G);
  p->insertSeparator();
  p->insertItem(i18n("&Properties"), view, SLOT(properties()));
  p->insertItem(i18n("&Delete"), view, SLOT(clear()), Key_Delete);
  p->insertSeparator();
  p->insertItem(i18n("&Quit"), kapp, SLOT(quit()), CTRL+Key_Q);
  
  edit = new QPopupMenu;
  undoId = edit->insertItem(i18n("Undo"), this, SLOT(undo()));
  redoId = edit->insertItem(i18n("Redo"), this, SLOT(redo()));
  edit->insertSeparator();
  edit->insertItem(i18n("Cut"), view, SLOT(cut()), CTRL+Key_X);
  edit->insertItem(i18n("Copy"), view, SLOT(copy()), CTRL+Key_C);
  edit->insertItem(i18n("Paste"), view, SLOT(paste()), CTRL+Key_V);
  edit->insertSeparator();
  edit->insertItem(i18n("Select All"), view, SLOT(selectAll()), CTRL+Key_A);
  edit->insertSeparator();
  edit->insertItem(i18n("&Find"), this, SLOT(find()), CTRL+Key_F);
  edit->setItemEnabled( undoId, false );
  edit->setItemEnabled( redoId, false );
  QObject::connect( edit, SIGNAL( aboutToShow() ), this, SLOT( updateEditMenu() ));

  QPopupMenu* v = new QPopupMenu;
  v->insertItem(i18n("Choose Fields..."), view, SLOT(showSelectNameDialog()) );
  v->insertItem(i18n("Options..."), view, SLOT(viewOptions()) );
  v->insertSeparator();
  v->insertItem(i18n("Restore defaults"), view, SLOT(defaultSettings()) );
  v->insertSeparator();
  v->insertItem(i18n("Refresh"), view, SLOT(refresh()), Key_F5 );

  // put our newly created menu into the main menu bar
  menuBar()->insertItem(i18n("&File"), p);
  menuBar()->insertItem(i18n("&Edit"), edit);
  menuBar()->insertItem(i18n("&View"), v);
  menuBar()->insertSeparator();

  // KDE will generate a short help menu automagically
  p = helpMenu( i18n("Kab --- KDE Address Book\n\n"
		     "(c) 1999 The KDE PIM Team \n"
		     "Long Description"));
  menuBar()->insertItem(i18n("&Help"), p);
  
  // insert a quit button.  the icon is the standard one in KDE
  toolBar()->insertButton(BarIcon("exit"),   // icon
			  0,                  // button id
			  SIGNAL(clicked()),  // action
			  kapp, SLOT(quit()), // result
			  i18n("Exit"));      // tooltip text
	
  toolBar()->insertButton(BarIcon("filenew"),   // icon
			  0,                  // button id
			  SIGNAL(clicked()),  // action
			  this, SLOT(newContact()), // result
			  i18n("Add a new entry"));      // tooltip text
  toolBar()->insertButton(BarIcon("page"),   // icon
			  0,                  // button id
			  SIGNAL(clicked()),  // action
			  view, SLOT(properties()), // result
			  i18n("Change this entry"));      // tooltip text
  toolBar()->insertButton(BarIcon("delete"),   // icon
			  0,                  // button id
			  SIGNAL(clicked()),  // action
			  view, SLOT(clear()), // result
			  i18n("Remove this entry"));      // tooltip text
  toolBar()->insertButton(BarIcon("find"),   // icon
			  0,                  // button id
			  SIGNAL(clicked()),  // action
			  this, SLOT(find()), // result
			  i18n("Search entries"));      // tooltip text

  toolBar()->insertButton(BarIcon("filemail"),   // icon
			  0,                  // button id
			  SIGNAL(clicked()),  // action
			  this, SLOT(mail()), // result
			  i18n("Send mail"));      // tooltip text

  
  // we do want a status bar
  enableStatusBar();
  connect( kapp, SIGNAL( aboutToQuit() ), this, SLOT( saveConfig() ) );
  setMinimumSize( sizeHint() );
}

void Pab::newContact()
{
 ContactDialog *test = new PabNewContactDialog( this, i18n( "Address Book Entry Editor" ));
 connect( test, SIGNAL( add( ContactEntry* ) ), 
	  view, SLOT( addNewEntry( ContactEntry* ) ));
 test->show();
}

void Pab::save()
{
  //xxx  document->save( "entries.txt" );
}

void Pab::readConfig()
{
}

void Pab::saveConfig()
{
  debug( "saveConfig" );
  view->saveConfig();
  KConfig *config = kapp->config();

  config->setGroup("Geometry");
  config->writeEntry("MainWin", "abc");
  config->sync();
}

Pab::~Pab()
{
  saveConfig();
  delete document;
}

void Pab::saveCe() {
  debug( "saveCe()" );
  //xxx  ce->save( "entry.txt" );
}

void Pab::saveProperties(KConfig *config)
{
	// the 'config' object points to the session managed
	// config file.  anything you write here will be available
	// later when this app is restored
  //what I want to save
  //windowsize
  //background image/underlining color/alternating color1,2
  //chosen fields
  //chosen fieldsWidths
  
	
	// e.g., config->writeEntry("key", var); 
}

void Pab::readProperties(KConfig *config)
{
	// the 'config' object points to the session managed
	// config file.  this function is automatically called whenever
	// the app is being restored.  read in here whatever you wrote
	// in 'saveProperties'

	// e.g., var = config->readEntry("key"); 
}

void Pab::undo()
{
  debug( "Pab::undo()" );
  UndoStack::instance()->undo();
}

void Pab::redo()
{
  RedoStack::instance()->redo();
}

void Pab::updateEditMenu()
{
  debug( "UpdateEditMenu()" );
  UndoStack *undo = UndoStack::instance();
  RedoStack *redo = RedoStack::instance();

  if (undo->isEmpty())
    edit->changeItem( undoId, i18n( "Undo" ) );
  else
    edit->changeItem( undoId, i18n( "Undo" ) + " " + undo->top()->name() );
  edit->setItemEnabled( undoId, !undo->isEmpty() );

  if (redo->isEmpty())
    edit->changeItem( redoId, i18n( "Redo" ) );
  else
    edit->changeItem( redoId, i18n( "Redo" ) + " " + redo->top()->name() );
  edit->setItemEnabled( redoId, !redo->isEmpty() );  
}
