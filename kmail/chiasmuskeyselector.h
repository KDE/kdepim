#ifndef CHIASMUSKEYSELECTOR_H
#define CHIASMUSKEYSELECTOR_H

#include <kdialogbase.h>
class KListBox;
class KLineEdit;
class QLabel;

class ChiasmusKeySelector : public KDialogBase
{
  Q_OBJECT

public:
  ChiasmusKeySelector( TQWidget* parent, const TQString& caption,
                       const TQStringList& keys, const TQString& currentKey,
                       const TQString& lastOptions );

  TQString key() const;
  TQString options() const;

private:
  TQLabel* mLabel;
  KListBox* mListBox;
  KLineEdit* mOptions;
};

#endif
