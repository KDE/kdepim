#include <qobject.h>

class OpieSocket : public QObject
{
 public:
  OpieSocket(QObject *obj, const char *name );
  void setUser(const QString &user );
  void setPass(const QString &pass );
  bool startSync();

};
