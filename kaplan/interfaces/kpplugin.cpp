#include "kpcore.h"


#include "kpplugin.h"


class Kaplan::Plugin::Private
{
public:

  Kaplan::Core *core;

};


Kaplan::Plugin::Plugin(Kaplan::Core *core, QObject *parent, const char *name)
  : QObject(parent, name)
{
  d = new Kaplan::Plugin::Private;

  d->core = core;
}


Kaplan::Plugin::~Plugin()
{
  delete d;
}


Kaplan::Core *Kaplan::Plugin::core() const
{
  return d->core;
}


#include "kpplugin.moc"
