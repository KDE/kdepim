#ifndef MOBILEGUI_H
#define MOBILEGUI_H

#include "mobilegui_base.h"

class CommandScheduler;
class ATCommand;

class MobileGui : public MobileGui_base
{ 
    Q_OBJECT
  public:
    MobileGui(CommandScheduler *,QWidget* parent=0,const char* name=0,
              bool modal=FALSE,WFlags fl=0);
    ~MobileGui();

  signals:
    void sendCommand(const QString &);
    void phonebookRead();

  public slots:
    void readModelInformation();
    void readPhonebook();
    void savePhonebook();
    void refreshStatus();
    void writePhonebook();
    void readKab();
    void writeKab();

  protected slots:
    void processResult(ATCommand *);

  private:
    void fillPhonebook(ATCommand *);
    QString dequote(const QString &);
  
    CommandScheduler *mScheduler;
};

#endif // MOBILEGUI_H
