
#include <qptrlist.h>
#include <kparts/mainwindow.h>

#include <manipulatorpart.h>

class PartBar;
class QHBox;

namespace KitchenSync {
  // no idea why we have this window
  class KSyncMainWindow : public KParts::MainWindow /*, public KSyncInterface */
    {
      Q_OBJECT
    public:
      KSyncMainWindow(QWidget *widget =0l, const char *name = 0l, WFlags f = WType_TopLevel );
      ~KSyncMainWindow();

    private:
      virtual void initActions();
      void addModPart( ManipulatorPart * );
      PartBar *m_bar;
      QHBox *m_lay;
      QPtrList<ManipulatorPart> m_parts;
      
    private slots:
      void initPlugins();
      void slotSync();
      void slotBackup();
      void slotRestore();
      void slotConfigure();
  };

  
};
