#ifndef CRLVIEW_H
#define CRLVIEW_H

#include <qdialog.h>

class QTextView;
class QPushButton;
class KProcess;

class CRLView : public QDialog {
  Q_OBJECT
public:
  CRLView( QWidget* parent = 0, const char* name = 0, bool modal = false );
  ~CRLView();
public slots:
  void slotUpdateView();

protected slots:
  void slotReadStdout( KProcess*, char* buf, int len);
  void slotProcessExited();

private:  
  QTextView*   _textView;
  QPushButton* _updateButton;
  QPushButton* _closeButton;
  KProcess*    _process;
};

#endif // CRLVIEW_H
