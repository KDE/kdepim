#ifndef __KP_CORE_H__
#define __KP_CORE_H__


#include <qstring.h>
#include <qobject.h>


#include <kparts/part.h>


namespace Kaplan
{


class Core
{
public:

  Core();
  virtual ~Core();

  virtual void addMainEntry(QString text, QString icon, QObject *reveiver, const char *slot) = 0;
  
  virtual void addPart(KParts::Part *part) = 0;

  virtual void showView(QWidget *view) = 0;

};


}


#endif

