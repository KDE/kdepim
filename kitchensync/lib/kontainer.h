
#ifndef kontainer_h
#define kontainer_h

#include <qstring.h>

class Kontainer {
public:
    Kontainer(const QString& = QString::null,
              const QString& = QString::null );
    Kontainer(const Kontainer & );
    ~Kontainer();
    QString first()const;
    QString second()const;
    Kontainer &operator=( const Kontainer& );
private:
    class KontainerPrivate;
    KontainerPrivate *d;
    QString m_first;
    QString m_second;
};

#endif
