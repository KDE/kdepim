#ifndef AKONADIDUMPER_H
#define AKONADIDUMPER_H

#include <QtCore/QList>

#include "abstractdump.h"

class AkonadiDump : public AbstractDump
{
  Q_OBJECT

public:
  explicit AkonadiDump( QString path, QObject *parent = 0 );

public slots:
  virtual void dump();
  virtual void restore();

private:
  void initializeResources( AbstractDump::Action action );
  QList<AbstractDump*> m_resources;
};

#endif // AKONADIDUMPER_H
