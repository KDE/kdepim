
#include <qregexp.h>

#include "desktop.h"

using namespace OpieHelper;

Desktop::Desktop( CategoryEdit* edit )
    : Base( edit ) {


};
Desktop::~Desktop() {

}
KSync::OpieDesktopSyncee* Desktop::toSyncee( const QString& str) {
    KSync::OpieDesktopSyncee* syncee;
    syncee = new KSync::OpieDesktopSyncee();

    // convert the string
    QString string ( str );
    string.remove(0, 35 );
    string.replace(QRegExp("&amp;"), "&" );
    string.replace(QRegExp("&0x20;"), " ");
    string.replace(QRegExp("&0x0d;"), "\n");
    string.replace(QRegExp("&0x0a;"), "\r");
    string.replace(QRegExp("\r\n"), "\n" ); // hell we're on unix

    if ( !str.contains("[Desktop Entry]") ) {
        kdDebug(5224) <<"Desktop Entry: " << str << endl;
        delete syncee;
        return 0l;
    }
    QStringList list = QStringList::split('\n', string );
    QStringList::Iterator it;
    it = list.begin();
    list.remove( it ); // remove the [Desktop Entry]

    KSync::OpieDesktopSyncEntry *entry;
    QString name,  type,  fileName,  size;
    QStringList category;

    for ( it = list.begin(); it != list.end(); ++it ) { // QAsciiDict?
        QString con( (*it).stripWhiteSpace() );

        if (con.startsWith("Categories = ") ) {
            con = con.remove(0, 13 );
            //con = con.remove( con.length() -1, 1 ); // remove the last char ;
            QStringList catList = QStringList::split(';',con ); // no empty cats
            category = edit()->categoriesByIds( catList, QString::null );

        }else if ( con.startsWith("Name = ") ) {
            con = con.remove(0, 7 );
            name = con.stripWhiteSpace();
        }else if ( con.startsWith("Type = ") ) {
            con = con.remove( 0, 7 );
            type = con.stripWhiteSpace() ;
        }else if ( con.startsWith("File = ") ) {
            con = con.remove( 0, 7 );
            fileName = con.stripWhiteSpace();
        }else if ( con.startsWith("Size = ") ) {
            con = con.remove(0, 7 );
            size = con.stripWhiteSpace();
        }else if ( (*it).stripWhiteSpace() == "[Desktop Entry]" ) {
            entry= new KSync::OpieDesktopSyncEntry( category,
                                                    fileName,
                                                    name,
                                                    type,
                                                    size );
            syncee->addEntry( entry );
        }

    }
    // one is missing
    entry = new KSync::OpieDesktopSyncEntry( category,
                                             fileName,
                                             name,
                                             type,
                                             size );
    syncee->addEntry( entry );
    return syncee;
}

