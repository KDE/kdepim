#ifndef KCAL_PERSON_H
#define KCAL_PERSON_H

#include <qstring.h>

namespace KCal {

class Person
{
  public:
    Person() {}
    Person( const QString &fullName );
    Person( const QString &name, const QString &email );

    QString fullName( ) const;

    void setName(const QString &);
    QString name() const { return mName; }
    
    void setEmail(const QString &);
    QString email() const { return mEmail; }

  private:
    QString mName;
    QString mEmail;
};

}

#endif
