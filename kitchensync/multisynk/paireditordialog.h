#ifndef PAIREDITORDIALOG_H
#define PAIREDITORDIALOG_H

#include <kdialogbase.h>

class PairEditorWidget;
class KonnectorPair;

class PairEditorDialog : public KDialogBase
{
  Q_OBJECT

  public:
    PairEditorDialog( QWidget *parent = 0, const char *name = 0 );
    ~PairEditorDialog();

    void setPair( KonnectorPair *pair );
    KonnectorPair *pair() const;

  private:
    void initGUI();
    PairEditorWidget *mPairEditorWidget;
};

#endif
