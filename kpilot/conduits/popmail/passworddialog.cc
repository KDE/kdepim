// passworddialog.cc
//
// Copyright (C) is unclear. Given that the comments are
// in German, I don't think Dan wrote this. The .h file
// is (C) 1997 Micael Roth.
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$

// This is an old trick so you can determine what revisions
// make up a binary distribution.
//
//
static char *id="$Id$";





#include <string.h>
#include <stdio.h>

#include <qapp.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qaccel.h>
#include <qmsgbox.h>
#include <qchkbox.h>

#include "passworddialog.h"
#include "passworddialog.moc"

#include <klocale.h>
#include <kapp.h>

PasswordDialog::PasswordDialog(const char *head, QWidget* parent, const char* name, bool modal, WFlags wflags)
   : QDialog(parent, name, modal, wflags)
{
    
   _head = head;

   //
   // Bei Bedarf einen kleinen Kommentar als Label einfuegen
   //
   if (_head)
   {
      QLabel *l;
      
      l = new QLabel(_head, this);
      l->setGeometry( 10, 10, 200, 20 );
   }
   
   //
   // Die eine oder zwei Zeile(n) mit der Passwortabfrage
   //
   QLabel *l_password = new QLabel(klocale->translate("Password"), this);
   l_password->setGeometry( 10, 40, 80, 30 );
   
   _w_password = new QLineEdit( this );
   _w_password->setGeometry( 90, 40, 100, 30 );
   _w_password->setEchoMode( QLineEdit::Password );
   
   //
   // Connect vom LineEdit herstellen und Accelerator
   //
   QAccel *ac = new QAccel(this);
   ac->connectItem( ac->insertItem(Key_Escape), this, SLOT(reject()) );
   
   connect( _w_password, SIGNAL(returnPressed()), SLOT(accept()) );
   
   //
   // Eine vertikale Linie erzeugen
   //
   QFrame *f = new QFrame(this);
   f->setLineWidth(1);
   f->setMidLineWidth(1);
   f->setFrameStyle( QFrame::HLine|QFrame::Raised);
   f->setGeometry( 10, 80, 180, 2 );
   
   //
   // Die Buttons "Ok" & "Cancel" erzeugen
   //
   QPushButton *b1, *b2;
   b1 = new QPushButton(klocale->translate("Ok"), this);
   b1->setGeometry( 10, 90, 80, 30 );
   
   b2 = new QPushButton(klocale->translate("Cancel"), this);
   b2->setGeometry( 110, 90, 80, 30 );
   
   // Buttons mit Funktionaliataet belegen
   connect( b1, SIGNAL(clicked()), SLOT(accept()) );
   connect( b2, SIGNAL(clicked()), SLOT(reject()) );
   
   // Fenstertitel
   setCaption(klocale->translate("Password"));
   
   // Focus
   _w_password->setFocus();
   
   setGeometry( x(), y(), 200, 130 );

}

const char * PasswordDialog::password()
{
   if ( _w_password )
      return _w_password->text();
   else
      return "";
}



