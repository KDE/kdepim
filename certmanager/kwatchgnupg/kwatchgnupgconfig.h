#ifndef KWATCHGNUPGCONFIG_H
#define KWATCHGNUPGCONFIG_H

#include <kdialogbase.h>

class QCheckBox;
class QSpinBox;
class KURLRequester;

class KWatchGnuPGConfig : public KDialogBase {
  Q_OBJECT
public:
  KWatchGnuPGConfig( QWidget* parent, const char* name = 0 );

  void loadConfig();
  void saveConfig();

signals:
  void reconfigure();
public slots:
  void slotChanged();
  void slotSave();

private:
  KURLRequester* mExeED;
  KURLRequester* mSocketED;
  QSpinBox* mLoglenSB;
  QCheckBox* mWordWrapCB;
};

#endif /* KWATCHGNUPGCONFIG_H */

