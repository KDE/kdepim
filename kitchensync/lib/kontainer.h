
#ifndef KONTAINER_H
#define KONTAINER_H

#include <qstring.h>
#include <qvaluelist.h>

class Kontainer {
public:
	/**
	 * Convinience typedef
         */
    typedef QValueList<Kontainer> ValueList;

    friend bool operator== ( const Kontainer &a ,  const Kontainer &b );
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
