#ifndef __KP_FACTORY_H__
#define __KP_FACTORY_H__


#include <klibloader.h>
#include <kinstance.h>


namespace Kaplan
{


class Plugin;
class Core;


class Factory : public KLibFactory
{
    Q_OBJECT  
  public:          
    Factory(QObject *parent=0, const char *name=0);
    ~Factory();

    Plugin *create(Core *core, QObject *parent, const QStringList &args=QStringList());

  protected:
    virtual Plugin *createPlugin(Core *core, QObject *parent, const QStringList &args) = 0;

    virtual QObject* createObject( QObject* parent = 0, const char* name = 0,
                                   const char* classname = "QObject",
                                   const QStringList &args = QStringList() );
};


template <class plugin>
class FactoryImpl : public Factory
{
  public:
    FactoryImpl(QObject *parent=0, const char *name=0);

    virtual Plugin *createPlugin(Core *core, QObject *parent, const QStringList &args);

    static KInstance *instance(QCString cls);

  private:
    static KInstance *s_instance; 
};


template <class plugin>
FactoryImpl<plugin>::FactoryImpl(QObject *parent, const char *name)
  : Factory(parent, name)
{
}


template <class plugin>
Plugin *FactoryImpl<plugin>::createPlugin(Core *core, QObject *parent, const QStringList &)
{
  return new plugin(core, parent);
}


template <class plugin>
KInstance *FactoryImpl<plugin>::s_instance = 0;


template <class plugin>
KInstance *FactoryImpl<plugin>::instance(QCString cls)
{
  if (!s_instance)
    s_instance = new KInstance(cls);

  return s_instance;
}


}


#endif
