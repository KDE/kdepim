

#include <kparts/mainwindow.h>

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
      PartBar *m_bar;
      QHBox *m_lay;
    };
};
