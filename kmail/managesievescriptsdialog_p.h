#ifndef __KMAIL__MANAGESIEVESCRIPTSDIALOG_P_H__
#define __KMAIL__MANAGESIEVESCRIPTSDIALOG_P_H__

#include <kdialogbase.h>

#include <tqtextedit.h>

namespace KMail {

class SieveEditor : public KDialogBase {
  Q_OBJECT
  Q_PROPERTY( TQString script READ script WRITE setScript )
public:
  SieveEditor( TQWidget * parent=0, const char * name=0 );
  ~SieveEditor();

  TQString script() const { return mTextEdit->text(); }
  void setScript( const TQString & script ) { mTextEdit->setText( script ); }
private slots:
  void slotTextChanged();
private:
  TQTextEdit * mTextEdit;
};

}

#endif /* __KMAIL__MANAGESIEVESCRIPTSDIALOG_P_H__ */

