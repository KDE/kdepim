/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sieveincludewidget.h"

#include <KPushButton>

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>


namespace KSieveUi {
static int MINIMUMINCLUDEACTION = 0;
static int MAXIMUMINCLUDEACTION = 8;

SieveIncludeLocation::SieveIncludeLocation(QWidget *parent)
    : KComboBox(parent)
{
    initialize();
}

SieveIncludeLocation::~SieveIncludeLocation()
{
}

void SieveIncludeLocation::initialize()
{

}

SieveIncludeActionWidget::SieveIncludeActionWidget(QWidget *parent)
    : QWidget(parent)
{
    initWidget();
}

SieveIncludeActionWidget::~SieveIncludeActionWidget()
{

}

void SieveIncludeActionWidget::generatedScript(QString &script, QStringList &requires)
{
    //TODO
}

void SieveIncludeActionWidget::initWidget()
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins( 0, 0, 0, 0 );

    mAdd = new KPushButton( this );
    mAdd->setIcon( KIcon( QLatin1String("list-add") ) );
    mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    mRemove = new KPushButton( this );
    mRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
    mRemove->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    mLayout->addWidget( mAdd, 1, 3 );
    mLayout->addWidget( mRemove, 1, 4 );

    connect( mAdd, SIGNAL(clicked()),
             this, SLOT(slotAddWidget()) );
    connect( mRemove, SIGNAL(clicked()),
             this, SLOT(slotRemoveWidget()) );
}

void SieveIncludeActionWidget::slotAddWidget()
{
    emit addWidget( this );
}

void SieveIncludeActionWidget::slotRemoveWidget()
{
    emit removeWidget( this );
}

void SieveIncludeActionWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
    mAdd->setEnabled(addButtonEnabled);
    mRemove->setEnabled(removeButtonEnabled);
}


SieveIncludeWidget::SieveIncludeWidget(QWidget *parent)
    : QWidget(parent)
{
}

SieveIncludeWidget::~SieveIncludeWidget()
{

}

SieveIncludeWidgetLister::SieveIncludeWidgetLister(QWidget *parent)
    : KPIM::KWidgetLister(false, MINIMUMINCLUDEACTION, MAXIMUMINCLUDEACTION, parent)
{
    slotClear();
    updateAddRemoveButton();
}

SieveIncludeWidgetLister::~SieveIncludeWidgetLister()
{

}

void SieveIncludeWidgetLister::slotAddWidget( QWidget *w )
{
    addWidgetAfterThisWidget( w );
    updateAddRemoveButton();
}

void SieveIncludeWidgetLister::slotRemoveWidget( QWidget *w )
{
    removeWidget( w );
    updateAddRemoveButton();
}


void SieveIncludeWidgetLister::updateAddRemoveButton()
{
    QList<QWidget*> widgetList = widgets();
    const int numberOfWidget( widgetList.count() );
    bool addButtonEnabled = false;
    bool removeButtonEnabled = false;
    if ( numberOfWidget <= widgetsMinimum() ) {
        addButtonEnabled = true;
        removeButtonEnabled = false;
    } else if ( numberOfWidget >= widgetsMaximum() ) {
        addButtonEnabled = false;
        removeButtonEnabled = true;
    } else {
        addButtonEnabled = true;
        removeButtonEnabled = true;
    }
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        SieveIncludeActionWidget *w = qobject_cast<SieveIncludeActionWidget*>( *wIt );
        w->updateAddRemoveButton( addButtonEnabled, removeButtonEnabled );
    }
}

void SieveIncludeWidgetLister::generatedScript(QString &script, QStringList &requires)
{
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        SieveIncludeActionWidget *w = qobject_cast<SieveIncludeActionWidget*>( *wIt );
        w->generatedScript(script, requires);
    }
}

void SieveIncludeWidgetLister::reconnectWidget(SieveIncludeActionWidget *w )
{
    connect( w, SIGNAL(addWidget(QWidget*)),
             this, SLOT(slotAddWidget(QWidget*)), Qt::UniqueConnection );
    connect( w, SIGNAL(removeWidget(QWidget*)),
             this, SLOT(slotRemoveWidget(QWidget*)), Qt::UniqueConnection );
}

void SieveIncludeWidgetLister::clearWidget( QWidget *aWidget )
{
    //TODO
}

QWidget *SieveIncludeWidgetLister::createWidget( QWidget *parent )
{
    SieveIncludeActionWidget *w = new SieveIncludeActionWidget( parent);
    reconnectWidget( w );
    return w;
}

}

#include "sieveincludewidget.moc"
