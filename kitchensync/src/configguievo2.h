#ifndef CONFIGGUIEVO2_H
#define CONFIGGUIEVO2_H

#include "configgui.h"

class KURLRequester;

class ConfigGuiEvo2 : public ConfigGui
{
  public:
    ConfigGuiEvo2( const QSync::Member &, QWidget *parent );

    void load( const QString &xml );

    QString save();

  private:
    void initGUI();

    KURLRequester *mAddressPath;

    KURLRequester *mCalendarPath;

    KURLRequester *mTasksPath;
};

#endif
