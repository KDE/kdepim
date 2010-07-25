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

#include "specialdatewidget.h"

#include <Plasma/Svg>
#include <Plasma/Theme>
#include <Plasma/DataEngine>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Service>

#include <akonadi/itemfetchjob.h>
#include <urihandler.h>

#include <KIcon>
#include <KUrl>

#include <QGraphicsLinearLayout>
#include <QDate>

SpecialDateWidget::SpecialDateWidget(QGraphicsWidget* parent, QString text, QString iconName, KUrl uri, QDate date )
    : Plasma::Frame(parent),
      m_date(date),
      m_icon(0),
      m_uri(uri),
      m_layout(0)
{    
    setMinimumHeight(40);
    setMinimumWidth(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // create icon
    KIcon ic = KIcon(iconName);
    
    setIcon(new Plasma::IconWidget( ic, "" ));
    m_layout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    
    m_layout->addItem( icon() );
    
    // create text    
    Plasma::Label* label = new Plasma::Label();
    label->setText(text);
    m_layout->addItem( label );
    
    // create daysTo
    QDate currentDate = QDate::currentDate();
    
    int daysTo = currentDate.daysTo(date);
    
    Plasma::BusyWidget* dayLabel = new Plasma::BusyWidget();
    QString dateString = QString("<p style='%2'>%1</p>").arg(daysTo).arg( daysTo > 1 ? "" : "background-color:red;");
    //dayLabel->setScaledContents(true);
    dayLabel->setLabel(dateString);
    m_layout->addItem( dayLabel );

    
    // Cleanup the layout
    m_layout->setAlignment(icon(),Qt::AlignLeft);
    m_layout->setAlignment(label,Qt::AlignLeft);
    m_layout->setAlignment(dayLabel,Qt::AlignLeft);
    
    connect(icon(), SIGNAL(clicked()), this, SLOT(click()));
    
}

void SpecialDateWidget::click()
{
    kDebug() << "Processing" << uri();

    uint id = uri().url().split(':')[1].toInt();
    kDebug() << id;
    Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob( Akonadi::Item( id ) );
    connect( job, SIGNAL( result( KJob* )), this, SLOT( fetchFinished( KJob* )) );
}

void SpecialDateWidget::fetchFinished( KJob* job )
{
    Akonadi::Item item;
    Akonadi::ItemFetchJob* fetchJob = qobject_cast<Akonadi::ItemFetchJob*>(job);
    if( fetchJob->items().length() > 0 ) {
        item = fetchJob->items().first();
    }
    UriHandler::process( uri().url(), item );
}

Plasma::IconWidget* SpecialDateWidget::icon()
{
    return m_icon;
}

void SpecialDateWidget::setIcon(Plasma::IconWidget* icon)
{
    m_icon = icon;
}

QDate SpecialDateWidget::date()
{
    return m_date;
}

void SpecialDateWidget::setDate(QDate date)
{
    m_date = date;
}

KUrl SpecialDateWidget::uri()
{
    return m_uri;
}

void SpecialDateWidget::setUri(KUrl uri)
{
    m_uri = uri;
}


#include "specialdatewidget.moc"
