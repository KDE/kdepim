#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <kdialogbase.h>

namespace KPIM {
class ProgressItem;
}

class QTextEdit;

class LogDialog : public KDialogBase
{
  Q_OBJECT

  public:
    LogDialog( QWidget *parent );

  private slots:
    void progressItemAdded( KPIM::ProgressItem* );
    void progressItemStatus( KPIM::ProgressItem*, const QString& );

  private:
    void initGUI();
    void log( const QString& );

    QTextEdit *mView;
};

#endif
