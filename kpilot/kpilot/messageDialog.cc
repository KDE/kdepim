// messageDialog.cc
//
// Copyright (C) 1998, 1999 Dan Pilone
//
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
//
static const char *id="$Id$";

#include <qdialog.h>
#include <kapp.h>
#include "messageDialog.moc"

MessageDialog::MessageDialog( QString title, QWidget* parent, const char* name, bool modal)
  : QDialog(parent, name, modal, 0)
    {
    setGeometry(x(), y(), 250, 40);
    setCaption(title);
    fMessage = new QLabel(title, this);
    fMessage->setFixedWidth(220);
    fMessage->move(10, 10);
    fMessage->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    fMessage->setAlignment(AlignBottom | AlignCenter);
    kapp->processEvents();
    }
  
void 
MessageDialog::setMessage(QString message)
    {
    fMessage->setText(message);
    fMessage->adjustSize();
    kapp->processEvents();
    }
