#ifndef KNCONFIGMANAGER_H
#define KNCONFIGMANAGER_H

#include "knconfig.h"

class KNConfigDialog;

class KNConfigManager : QObject {

  Q_OBJECT

  public:
    KNConfigManager(QObject *p=0, const char *n=0);
    ~KNConfigManager();


    KNConfig::Identity*             identity()            { return i_dentity; }
    KNConfig::Appearance*           appearance()          { return a_ppearance; }
    KNConfig::ReadNewsGeneral*      readNewsGeneral()     { return r_eadNewsGeneral; }
    KNConfig::DisplayedHeaders*     displayedHeaders()    { return d_isplayedHeaders; }
    KNConfig::PostNewsTechnical*    postNewsTechnical()   { return p_ostNewsTechnical; }
    KNConfig::PostNewsComposer*     postNewsComposer()    { return p_ostNewsCompose; }
    KNConfig::Cleanup*              cleanup()             { return c_leanup; }

    void configure();


  protected:
    KNConfig::Identity             *i_dentity;
    KNConfig::Appearance           *a_ppearance;
    KNConfig::ReadNewsGeneral      *r_eadNewsGeneral;
    KNConfig::DisplayedHeaders     *d_isplayedHeaders;
    KNConfig::PostNewsTechnical    *p_ostNewsTechnical;
    KNConfig::PostNewsComposer     *p_ostNewsCompose;
    KNConfig::Cleanup              *c_leanup;

    KNConfigDialog  *d_ialog;


  protected slots:
    void slotDialogDone();

};


class KNConfigDialog : public KDialogBase {

  Q_OBJECT

  public:
    KNConfigDialog(KNConfigManager *m, QWidget *p=0, const char *n=0);
    ~KNConfigDialog();

  protected:
    QList<KNConfig::BaseWidget> w_idgets;

  protected slots:
    void slotApply();
    void slotOk();



};



#endif //KNCONFIGMANAGER_H




