// $Id$

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
