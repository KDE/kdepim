/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef ADDRESSEEEDITORDIALOG_H
#define ADDRESSEEEDITORDIALOG_H

#include <kdialogbase.h>

#include <kabc/addressbook.h>

class AddresseeEditorBase;
class QWidget;
namespace KAB { class Core; }

class AddresseeEditorDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AddresseeEditorDialog( KAB::Core *core,
                           QWidget *parent, const char *name = 0 );
    ~AddresseeEditorDialog();

    void setAddressee( const KABC::Addressee& );
    KABC::Addressee addressee();

    bool dirty();

  signals:
    void contactModified( const KABC::Addressee& );
    void editorDestroyed( const QString& );

  protected slots:
    virtual void slotApply();
    virtual void slotOk();
    virtual void slotCancel();
    void widgetModified( const KABC::Addressee::List& );

  private:
    virtual void setTitle( const KABC::Addressee& );

    AddresseeEditorBase *mEditorWidget;
};

#endif
