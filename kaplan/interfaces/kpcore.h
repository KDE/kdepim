#ifndef __KP_CORE_H__
#define __KP_CORE_H__


#include <qstring.h>
#include <qobject.h>

#include <kparts/mainwindow.h>
#include <kparts/part.h>

class KAction;

namespace Kaplan
{


class Core : public KParts::MainWindow
{
protected:

  Core(QWidget *parentWidget = 0, const char *name = 0);
public:
  virtual ~Core();

  virtual void addMainEntry(QString text, QString icon, QObject *reveiver, const char *slot) = 0;
  
  virtual void addPart(KParts::Part *part) = 0;

  virtual void showView(QWidget *view) = 0;

  virtual void insertNewAction(KAction *action) = 0;

};


}


#endif

