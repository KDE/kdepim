/*
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */
#ifndef COMCFG_H
#define COMCFG_H

#include "moncfg.h"

class QLineEdit;

class KCommandsCfg : public KMonitorCfg
{
  Q_OBJECT

  public:

    KCommandsCfg(KMailDrop * drop) : KMonitorCfg(drop) {}
    virtual ~KCommandsCfg() {}

    virtual QString name() const;
    virtual QWidget * makeWidget(QWidget * parent);
    virtual void readConfig();
    virtual void updateConfig();

  private:

    QLineEdit *_onClick;
    QLineEdit *_onReceipt;
};

#endif // COMCFG_H
