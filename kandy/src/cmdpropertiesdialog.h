#ifndef CMDPROPERTIESDIALOG_H
#define CMDPROPERTIESDIALOG_H

#include "cmdpropertiesdialog_base.h"

class ATCommand;

class CmdPropertiesDialog : public CmdPropertiesDialog_base
{ 
    Q_OBJECT
  public:
    CmdPropertiesDialog(ATCommand *cmd,QWidget* parent=0,const char* name=0,
                        bool modal=false,WFlags fl=0);
    ~CmdPropertiesDialog();

  protected slots:
    void editParameterName(QListViewItem *);
    void slotAccept();

  private:
    void readCommand();
    void writeCommand();
        
    ATCommand *mCmd;
};

#endif // CMDPROPERTIESDIALOG_H
