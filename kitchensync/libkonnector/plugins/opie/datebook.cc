
#include <qdom.h>
#include <qfile.h>

#include <kalendarsyncentry.h>

#include "datebook.h"

using namespace OpieHelper;

DateBook::DateBook( CategoryEdit* edit,
                    KonnectorUIDHelper* helper,
                    bool meta )
    : Base( edit,  helper,  meta )
{


}
DateBook::~DateBook()
{

}
QPtrList<KCal::Event> DateBook::toKDE( const QString& fileName )
{
    QPtrList<KCal::Event> m_list;

    QFile file( fileName );
    if ( file.open( IO_ReadOnly ) ) {
        QDomDocument doc("mydocument");
        if ( doc.setContent( &file ) ) {
            KCal::Event *event;
            QDomElement docElem = doc.documentElement();
            QDomNode n = docElem.firstChild();
            QString dummy;
            int Int;
            bool ok;
            while (!n.isNull() ) {
                QDomElement e = n.toElement();
                if (!e.isNull() ) {
                    if (e.tagName() == "event") {
                        event = new KCal::Event();
                        QStringList list = QStringList::split(";",  e.attribute("Categories") );
                        QStringList categories;
                        for ( uint i = 0; i < list.count(); i++ ) {
                            categories.append(m_edit->categoryById(list[i], "Calendar") );
                        }
                        if (!categories.isEmpty() ) {
                            event->setCategories( categories );
                        }
                        //event->setDescription(e.attribute("Description") );
                        event->setSummary( e.attribute("description") );
                        event->setUid( kdeId( "event",  e.attribute("uid") ) );
                        event->setDescription( e.attribute("note") );
                        event->setLocation( e.attribute("location") );
                        // time
                        bool ok;
                        QString start = e.attribute("start");
                        event->setDtStart( fromUTC( (time_t) start.toLong() ) );
                        QString end = e.attribute("end");
                        event->setDtEnd( fromUTC( (time_t) end.toLong() ) );
                        if ( e.attribute("type") == "AllDay" ) {
                            event->setFloats( true );
                        }else{
                            event->setFloats( false );
                        }
                        KCal::Alarm *al = new KCal::Alarm( event );

                        m_list.append( event );
                    }
                    n = n.nextSibling();
                } // n.isNULL
            }
        }
    }
    return m_list;
}
QByteArray DateBook::fromKDE( KAlendarSyncEntry* entry )
{

}
