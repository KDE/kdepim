/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qlistview.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kinputdialog.h>
#include <klocale.h>

#include "atcommand.h"

#include "cmdpropertiesdialog.h"
#include "cmdpropertiesdialog.moc"

class ParameterItem : public QCheckListItem {
  public:
    ParameterItem(ATParameter *p,QListView *parent) :
        QCheckListItem(parent,p->name(),CheckBox),mParameter(p)
    {
      setText(1,p->value());
      setOn(p->userInput());
    }
        
    void writeParameter()
    {
      mParameter->setName(text(0));
      mParameter->setValue(text(1));
      mParameter->setUserInput(isOn());
    }
    
  private:
    ATParameter *mParameter;
};


/* 
 *  Constructs a CmdPropertiesDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CmdPropertiesDialog::CmdPropertiesDialog(ATCommand *cmd, QWidget* parent,
                                         const char* name, bool modal,
                                         WFlags fl )
    : CmdPropertiesDialog_base( parent, name, modal, fl )
{
  mCmd = cmd;
  
  readCommand();
}

CmdPropertiesDialog::~CmdPropertiesDialog()
{
}

void CmdPropertiesDialog::readCommand()
{
  mNameEdit->setText(mCmd->cmdName());
  mStringEdit->setText(mCmd->cmdString());
  mHexCheck->setChecked(mCmd->hexOutput());

  QPtrList<ATParameter> parameters = mCmd->parameters();
  for(int i=(int)parameters.count()-1;i>=0;--i) {
    ATParameter *p = parameters.at(i);
    new ParameterItem(p,mParameterList);
  }
}

void CmdPropertiesDialog::writeCommand()
{
  mCmd->setCmdName(mNameEdit->text());
  mCmd->setCmdString(mStringEdit->text());
  mCmd->setHexOutput(mHexCheck->isChecked());
  ParameterItem *item = (ParameterItem *)mParameterList->firstChild();
  while (item) {
    item->writeParameter();
    item = (ParameterItem *)item->nextSibling();
  }
}

void CmdPropertiesDialog::editParameterName(QListViewItem *item)
{
  bool ok = false;

  QString newName = KInputDialog::getText(QString::null,
                        i18n("Enter parameter name:"),item->text(0),&ok,this);

  if (ok) {
    item->setText(0,newName);
  }
}

void CmdPropertiesDialog::slotAccept()
{
  writeCommand();
  accept();
}
