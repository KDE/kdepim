#include <kdebug.h>
#include <klocale.h>

#include "person.h"

using namespace KCal;

Person::Person( const QString &fullName )
{
  int emailPos = fullName.find( '<' );
  if ( emailPos < 0 ) {
    setEmail(fullName);
  } else {
    setEmail(fullName.mid( emailPos + 1, fullName.length() - 1 ));
    setName(fullName.left( emailPos - 2 ));
  }
}

Person::Person( const QString &name, const QString &email )
{
  setName(name);
  setEmail(email);
}

QString Person::fullName() const
{
  if( mName.isEmpty() ) {
    if( mEmail.isEmpty() )
      return i18n( "Unknown" );
    else
      return mEmail;
  } else {
    if( mEmail.isEmpty() )
      return mName;
    else
      return mName + " <" + mEmail + ">";
  }
}

void Person::setName(const QString &name)
{
  mName = name;
}

void Person::setEmail(const QString &email)
{
  if (email.left(7).lower() == "mailto:") {
    mEmail = email.mid(7);
  } else {
    mEmail = email;
  }
}
