#ifndef CONFIGGUIMOTO_H
#define CONFIGGUIMOTO_H

#include "configgui.h"

class KLineEdit;

class ConfigGuiMoto : public ConfigGui
{
  public:
    ConfigGuiMoto( const QSync::Member &, QWidget *parent );

    void load( const QString &xml );

    QString save();

  private:
    void initGUI();

    KLineEdit *mDeviceString;
};

#endif
