#ifndef MOBILEGUI_H
#define MOBILEGUI_H

#include <kandyiface.h>

#include "mobilegui_base.h"

class CommandScheduler;
class ATCommand;
class AddressSyncer;

class MobileGui : public MobileGui_base, virtual public KandyIface
{ 
    Q_OBJECT
  public:
    MobileGui(CommandScheduler *,QWidget* parent=0,const char* name=0,
              WFlags fl=0);
    ~MobileGui();

    void exit();

  signals:
    void sendCommand(const QString &);
    void phonebookRead();

    void statusMessage(const QString &);
    void transientStatusMessage(const QString &);

  public slots:
    void readModelInformation();
    void readPhonebook();
    void savePhonebook();
    void refreshStatus();
    void writePhonebook();
    void readKab();
    void writeKab();
    void mergePhonebooks();
    void syncPhonebooks();

  protected slots:
    void processResult(ATCommand *);

  private:
    void fillPhonebook(ATCommand *);
    QString quote(const QString &);
    QString dequote(const QString &);
  
    void updateKabBook();
    void updateMobileBook();
    void updateCommonBook();
  
    CommandScheduler *mScheduler;
    
    AddressSyncer *mSyncer;
    
    QString mLastWriteId;
    QString mSyncReadId;
    QString mSyncWriteId;
    
    bool mSyncing;
};

#endif // MOBILEGUI_H
