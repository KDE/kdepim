#ifndef CONFIGGUISYNCE_H
#define CONFIGGUISYNCE_H

#include "configgui.h"

class KLineEdit;

class ConfigGuiSynce : public ConfigGui
{
  public:
    ConfigGuiSynce( const QSync::Member &, QWidget *parent );

    void load( const QString &xml );

    QString save();

  private:
    void initGUI();

    KLineEdit *mContacts;

    KLineEdit *mTodos;

    KLineEdit *mCalendar;

    KLineEdit *mFile;
};

#endif
