
#include "helper.h"

using namespace OpieHelper;

Base::Base( CategoryEdit* edit,
            KonnectorUIDHelper* helper,
            bool metaSyncing )
{
    m_metaSyncing = metaSyncing;
    m_edit = edit;
    m_helper = helper;
}
Base::~Base()
{

}
bool Base::isMetaSyncingEnabled()const
{
    return m_metaSyncing;
}
void Base::setMetaSyncingEnabled(bool meta )
{
    m_metaSyncing = meta;
}
QString Base::categoriesToNumber( const QStringList &list, const QString &app )
{
    QString dummy;
    QValueList<OpieCategories>::ConstIterator catIt;
    QValueList<OpieCategories> categories = m_edit->categories();
    bool found = false;
    for ( QStringList::ConstIterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
        for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
            if ( (*catIt).name() == (*listIt) ) { // the same name
                found= true;
                dummy.append( (*catIt).id() + ";");
            }
        }
        if ( !found )
         dummy.append( QString::number(m_edit->addCategory( app, (*listIt) ) ) + ";" );  // generate a new category
    }
    if ( !dummy.isEmpty() )
        dummy.remove(dummy.length() -1,  1 ); //remove the last ;
    return dummy;
}
QStringList Base::categoriesToNumberList( const QStringList &list, const QString &app )
{
    QStringList dummy;
    QValueList<OpieCategories>::ConstIterator catIt;
    QValueList<OpieCategories> categories = m_edit->categories();
    bool found = false;
    for ( QStringList::ConstIterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
        for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
            if ( (*catIt).name() == (*listIt) ) { // the same name
                found= true;
                dummy <<  (*catIt).id();
            }
        }
        if ( !found ) {
            dummy << QString::number( m_edit->addCategory(app, (*listIt) ) );
        }
    }
    return dummy;
}
QString Base::konnectorId( const QString &appName,  const QString &uid )
{
    QString id;
    // Konnector-.length() ==  10
    if ( uid.startsWith( "Konnector-" ) ) { // not converted
        id =  uid.mid( 11 );
    }else if ( m_helper) {
        id =  m_helper->konnectorId( appName,  uid );
        //                        konnector kde
        m_kde2opie.append( Kontainer( id,     uid ) );
    }

    return id;
}
