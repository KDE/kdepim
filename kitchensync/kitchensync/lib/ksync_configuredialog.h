
#ifndef KSYNC_CONFIGUREDIALOG
#define KSYNC_CONFIGUREDIALOG

#include <kdialogbase.h>

namespace KitchenSync {

  class ConfigureDialog : public KDialogBase {
    Q_OBJECT
    
      public:
    ConfigureDialog( QWidget *parent=0, const char *name=0, bool modal=true );
    ~ConfigureDialog();
    virtual void show();
    void addWidget();
   
    protected slots:
     virtual void slotOk();
     virtual void slotCancel();
     
  protected:
     /**
      * Plugin sensitive. 
      */
     void apply(bool);
  private:
     /**
      * load and registers the plugins
      */
     void ConfigureDialog::setup();
     
     /**
      * unload the plugins
      */
     void ConfigureDialog::unload();
     
  };
  

};
#endif
