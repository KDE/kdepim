#ifndef __KP_PLUGIN_H__
#define __KP_PLUGIN_H__


#include <qobject.h>


#include <kxmlguiclient.h>


namespace Kaplan
{


class Core;

        
class Plugin : public QObject, virtual public KXMLGUIClient
{
  Q_OBJECT

public:

  Plugin(Core *core, QObject *parent=0, const char *name=0);
  ~Plugin();

  Core *core() const;


private:

  class Private;

  Private *d;

};


}


#endif
