/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 * 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "specialdatesapplet.h"
#include "specialdatewidget.h"

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/DataEngine>
#include <Plasma/ScrollWidget>

#include <QPainter>
#include <QFontMetrics>
#include <QSizeF>
#include <QGraphicsLinearLayout>
#include <QDate>
#include <QTimer>

K_EXPORT_PLASMA_APPLET(special_dates, SpecialDatesApplet)

SpecialDatesApplet::SpecialDatesApplet( QObject* parent, QVariantList args )
    : Plasma::Applet( parent, args ),
      m_layout(0),
      m_svg(this),
      m_calEngine(0),
      m_akoEngine(0),
      m_numDays(31)
{
    Q_UNUSED(args);
    
    m_svg.setImagePath("widgets/background");
    setBackgroundHints(DefaultBackground);
    
}

void SpecialDatesApplet::init()
{
    setPreferredSize( 50, 300 );
    
    QGraphicsLinearLayout* lay = new QGraphicsLinearLayout(this);
    
    m_ItemsScroll= new Plasma::ScrollWidget(this);
    m_ItemsScroll->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_ItemsPage = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, m_ItemsPage);
    m_ItemsScroll->setWidget(m_ItemsPage);
    lay->addItem(m_ItemsScroll);

    setLayout(lay);

    m_calEngine = dataEngine("calendar");
    m_akoEngine = dataEngine("akonadi");
    connect(m_akoEngine, SIGNAL(sourceAdded(QString)), this, SLOT(addSource(QString)));
    
    // Load configuration
    configChanged();
    
    updateSpecialDates();
    m_akoEngine->connectSource("ContactCollections", this);
}

void SpecialDatesApplet::addSource(QString sourceName)
{
    //kDebug() << "Connecting to source" << sourceName;
    
    m_akoEngine->connectSource(sourceName, this);
}

void SpecialDatesApplet::paintInterface(QPainter* p,
                                     const QStyleOptionGraphicsItem* option, const QRect& contentsRect)
{
    Q_UNUSED(p)
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)
}

void SpecialDatesApplet::configChanged()
{
    // TODO: Configuration interface
    m_numDays = config().readEntry("numDays",m_numDays);
    m_locale = config().readEntry("locale", "us_en-us"); // TODO default to system locale?
}

void SpecialDatesApplet::updateSpecialDates()
{
    // construct the DataEngine query
    QDate date = QDate::currentDate();
    
    QString query = QString("holidays:%1:%2:%3");
    query = query.arg(m_locale, date.toString("yyyy-MM-dd"), date.addDays(m_numDays).toString("yyyy-MM-dd"));
    
    //kDebug() << "Query calendar DataSource" << query;
    
    // when did this change? rrix 20100613
    Plasma::DataEngine::Data holidayQuery = m_calEngine->query(query);
    QVariantList holidayList = holidayQuery[query].toList();
    if( holidayList.length() > 0 && holidayList.first().isValid() )
    {
        QHash<QString,QVariant> holidays = holidayList.first().toHash();
    
        // unpack the hash
        QListIterator<QVariant> it(holidayList);
        while( it.hasNext() )
        {
            QHash<QString,QVariant> data = it.next().toHash();
            QDate date = data["date"].toDate();
        
            QString text = data["name"].toString();
        
            kDebug() << date << text;

            QString key = data[ "date" ].toString();
            if (m_specialDates[ key ]) { // XXX maybe, need to validate this
                int i = 0;
                while ( i <= 255 ) { // XXX try moar?
                    i++;
                    key = key + "~";

                    if (m_specialDates[ key ]) { // XXX need to validate this works too
                        i = 255;
                    }
                }
            }
            m_specialDates[ key ] = new SpecialDateWidget(this,text,"view-calendar-holiday",KUrl(),date);
        }
    }
    else
    {
        QTimer::singleShot(60000, this, SLOT(updateSpecialDates()));
        kDebug() << "Setting singleshot";
    }
}


void SpecialDatesApplet::updateUI()
{
    //kDebug() << "Constructing interface based on m_specialDates";
    //kDebug() << m_specialDates;
    
    for( int i = 0; i < m_layout->count(); i++ )
    {
        m_layout->removeAt(i); //ugh.
    }
    
    QMapIterator<QString,Plasma::Frame*> it(m_specialDates);
    while( it.hasNext() )
    {
        it.next();
        //SpecialDate* widget = new SpecialDate(this, it.value() );
        m_layout->addItem(it.value());
    }

    setPreferredSize(-1,-1);
    m_layout->invalidate();

    emit sizeHintChanged(Qt::PreferredSize);
}

void SpecialDatesApplet::dataUpdated(const QString& sourceName, const Plasma::DataEngine::Data& data)
{
    // kDebug() << data;
    if( sourceName == "ContactCollections" )
    {
        // data.key() is the collection name
        QHashIterator<QString,QVariant> it(data);
        while( it.hasNext() )
        {
            it.next();
            QString query = it.key();
            
            //kDebug() << "Query Akonadi Engine for " << query;
            
            m_akoEngine->query(query);
            
            // At this point, a boatload of new Collections will appear as Sources.
            // Since the UI munges itself and I can't figure out a way to order the items in the layout in
            // a sane fashion, we're gonna wait until 1 sec after the (hopefully) last item appears.
            // This is really broken and is a big FIXME.
            m_updateTimer = new QTimer(this);
            m_updateTimer->setSingleShot(true);
            connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateUI()));
            m_updateTimer->start(1000);
        }
    }
    else if( sourceName.startsWith("Contact-"))
    {
        // individual contact entries
        QDate birthday = data.value("Birthday").toDate();
        
        if( !birthday.isNull() )
        {
            //kDebug() << birthday;
            
            QDate currentDate = QDate::currentDate();
            
            // make the QDate reference this year.
            QDate birthday2 = QDate(currentDate.year(), birthday.month(), birthday.day() );
            
            int daysTo = currentDate.daysTo(birthday2);
            
            if( daysTo >= 0 && daysTo <= m_numDays )
            {
                QString text = "%1's Birthday";
                text = text.arg( data["Name"].toString() );
                QString uri = "uid:%1";
                uri = uri.arg( data["Id"].toString( ) );
 
                QString key = birthday2.toString(Qt::ISODate);
                if (m_specialDates[ key ]) { // XXX maybe, need to validate this
                    int i = 0;
                    while ( i <= 255 ) { // XXX try moar?
                        i++;
                        key = key + "~";

                        if (m_specialDates[ key ]) { // XXX need to validate this works too
                            i = 255;
                        }
                    }
                }

                m_specialDates[ key ] = new SpecialDateWidget(this,text, "view-calendar-birthday", uri, birthday2);
                
                m_updateTimer->start(1000);
            }
        }
    }
}

#include "specialdatesapplet.moc"
