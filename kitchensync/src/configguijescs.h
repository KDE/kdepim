#ifndef CONFIGGUIJESCS_H
#define CONFIGGUIJESCS_H

#include "configgui.h"

class KLineEdit;
class QCheckBox;

class ConfigGuiJescs : public ConfigGui
{
  public:
    ConfigGuiJescs( const QSync::Member &, QWidget *parent );

    void load( const QString &xml );

    QString save() const;

  private:
    void initGUI();

    KLineEdit *mUrl;

    KLineEdit *mUsername;

    KLineEdit *mPassword;

    QCheckBox *mDelNotify;
};

#endif
