/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#ifndef KDE_VERSION
#include <kdeversion.h>
#endif

#if KDE_IS_VERSION(3,1,90)
#include <ktimewidget.h>
#else
#warning "Workaround for KTimeWidget in KDE 3.1"
class KTimeWidget : public QWidget
{
public:
	KTimeWidget(QWidget *p, const char *n) : QWidget(p,n) {};
} ;


#endif
