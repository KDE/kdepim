/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
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

#include "special_dates.h"
#include "specialdateswidget.h"

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/DataEngine>

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
      m_numDays(31),
      m_calEngine(0),
      m_akoEngine(0)
{
    Q_UNUSED(args);
    
    m_svg.setImagePath("widgets/background");
    setBackgroundHints(DefaultBackground);
    
    resize(500,200);
    
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    setLayout(m_layout);
}

void SpecialDatesApplet::init()
{
    m_calEngine = dataEngine("calendar");
    m_akoEngine = dataEngine("akonadi");
    connect(m_akoEngine, SIGNAL(sourceAdded(QString)), this, SLOT(addSource(QString)));
    
    // Load configuration
    configChanged();
    
    updateSpecialDates();
    
}

void SpecialDatesApplet::paintInterface(QPainter* p,
                                     const QStyleOptionGraphicsItem* option, const QRect& contentsRect)
{
    /*p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);
    
    p->save();
    p->setPen(Qt::white);
    p->drawText(contentsRect,
                Qt::AlignBottom | Qt::AlignHCenter,
                "Hello Plasmoid!");
    p->restore();
    */
}

void SpecialDatesApplet::configChanged()
{
    m_numDays = config().readEntry("numDays",m_numDays);
    m_locale = config().readEntry("locale", "us_en-us"); // TODO default to system locale?
}

void SpecialDatesApplet::updateSpecialDates()
{
    // construct the DataEngine query
    QDate date = QDate::currentDate();
    
    QString query = QString("holidays:%1:%2:%3");
    query = query.arg(m_locale, date.toString("yyyy-MM-dd"), date.addDays(m_numDays).toString("yyyy-MM-dd"));
    kDebug() << "Query calendar DataSource" << query;
    
    Plasma::DataEngine::Data holidays = m_calEngine->query(query);
    
    // unpack the hash
    QHashIterator<QString,QVariant> it(holidays);
    while( it.hasNext() )
    {
        it.next();
        QHash<QString,QVariant> data = it.value().toHash();
        
        m_specialDates[it.key()] = data["name"];
    }
    
    m_akoEngine->connectSource("ContactCollections", this);
    
    //updateUI();
}

void SpecialDatesApplet::addSource(QString sourceName)
{
    // kDebug() << "Connecting to source" << sourceName;
    
    m_akoEngine->connectSource(sourceName, this);
}

// TODO: Implement removeSource

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
            
            kDebug() << "Query Akonadi Engine for " << query;
            
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
            kDebug() << birthday;
            
            QDate currentDate = QDate::currentDate();
            
            // make the QDate reference this year.
            QDate birthday2 = QDate(currentDate.year(), birthday.month(), birthday.day() );
            // TODO Could I just daysTo % 365?
            
            int daysTo = currentDate.daysTo(birthday2);
            
            if( daysTo >= 0 && daysTo <= m_numDays )
            {
                kDebug() << daysTo;
                
                m_specialDates[birthday2.toString("yyyy-MM-dd")] = QString("%1's Birthday").arg( data.value("Name").toString() );
                
                m_updateTimer->start(1000);
            }
        }
    }
}

void SpecialDatesApplet::updateUI()
{
    kDebug() << "Constructing interface based on m_specialDates";
    kDebug() << m_specialDates;
    
    for( int i = 0; i < m_layout->count(); i++ )
    {
        m_layout->removeAt(i);
    }
    
    QMapIterator<QString,QVariant> it(m_specialDates);
    while( it.hasNext() )
    {
        it.next();
        SpecialDate* widget = new SpecialDate(it.key(), it.value().toString() );
        m_layout->addItem(widget);
    }
    
    //setLayout(m_layout);
}

#include "special_dates.moc"