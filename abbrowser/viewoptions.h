#ifndef VIEWOPTIONS_H 
#define VIEWOPTIONS_H 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <qdialog.h>
#include <qstring.h>
#include <qcolor.h>

class QLineEdit;
class QCheckBox;
class QRadioButton;
class KColorButton;

class ViewOptions : public QDialog
{
  Q_OBJECT

public:
  ViewOptions( bool backPixmapOn = false,
	       QString backPixmap = "",
	       bool underline = true,
	       bool autUnderline = true,
	       QColor cUnderline = QColor( 0, 0, 0),
	       bool tooltips = true,
	       QWidget * parent=0, 
	       const char * name=0, 
	       bool modal = false );

  QCheckBox *ckBackPixmap;
  QLineEdit *leBackPixmap;
  QCheckBox *ckUnderline;
  QRadioButton *ckAutUnderline;
  QRadioButton *ckManUnderline;
  KColorButton *kcbUnderline;
  QCheckBox *ckTooltips;

public slots:
  void pickPixmap();
  void pixmapOn();
  void underlineOn();
  void autUnderlineOff();
};

#endif // PABWIDGET_H 
