#ifndef KJOTSCONFIGDLG_H
#define KJOTSCONFIGDLG_H


#include <kcmodule.h>
#include <kcmultidialog.h>
#include "ui_confpagemisc.h"

class confPageMisc : public QDialog, public Ui::confPageMisc
{
public:
  confPageMisc( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};


class KJotsConfigMisc : public KCModule
{
  Q_OBJECT
  public:
  KJotsConfigMisc( const KComponentData &inst, QWidget *parent );
    /** Reimplemented from KCModule. */
    virtual void load();
    virtual void save();
};

class KJotsConfigDlg : public KCMultiDialog
{
  Q_OBJECT
  public:
  KJotsConfigDlg( const QString & title, QWidget *parent );
  ~KJotsConfigDlg();

public slots:
    void slotOk();
};

#endif /* KJOTSCONFIGDLG_H */

