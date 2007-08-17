#ifndef __KLEOPATRA_UTILS_FORMATTING_H__
#define __KLEOPATRA_UTILS_FORMATTING_H__

namespace GpgME {
    class Key;
    class UserID;
    class Subkey;
}

class QString;
class QDate;

namespace Kleo {
namespace Formatting {

    QString prettyName( int proto, const char * id, const char * name, const char * comment );
    QString prettyName( const GpgME::Key & key );
    QString prettyName( const GpgME::UserID & uid );


    QString prettyEMail( const GpgME::Key & key );
    QString prettyEMail( const GpgME::UserID & uid );


    QString toolTip( const GpgME::Key & key );


    QString expirationDateString( const GpgME::Key & key );
    QString expirationDateString( const GpgME::Subkey & subkey );
    QDate expirationDate( const GpgME::Key & key );
    QDate expirationDate( const GpgME::Subkey & subkey );

    QString creationDateString( const GpgME::Key & key );
    QString creationDateString( const GpgME::Subkey & subkey );
    QDate creationDate( const GpgME::Key & key );
    QDate creationDate( const GpgME::Subkey & subkey );


    QString type( const GpgME::Key & key );
    QString type( const GpgME::Subkey & subkey );


    QString validityShort( const GpgME::Subkey & subkey );
}
}

#endif /* __KLEOPATRA_UTILS_FORMATTING_H__ */
