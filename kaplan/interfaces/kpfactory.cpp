#include "kpfactory.h"
#include "kpplugin.h"


Kaplan::Factory::Factory(QObject *parent, const char *name)
  : KLibFactory(parent, name)
{
}


Kaplan::Factory::~Factory()
{
}


Kaplan::Plugin *Kaplan::Factory::create(Kaplan::Core *core, QObject *parent, const QStringList &args)
{
  Kaplan::Plugin *plugin  = createPlugin(core, parent, args);

  if (plugin)
    emit objectCreated(plugin);
  
  return plugin;
}

QObject* Kaplan::Factory::createObject( QObject* parent, const char* name,
                                        const char* classname,
                                        const QStringList &args )
{
  return 0;
}

#include "kpfactory.moc"
