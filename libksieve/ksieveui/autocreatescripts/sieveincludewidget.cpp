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

namespace KSieveUi {
static int MINIMUMINCLUDEACTION = 1;
static int MAXIMUMINCLUDEACTION = 8;
SieveIncludeActionWidget::SieveIncludeActionWidget(QWidget *parent)
    : QWidget(parent)
{

}

SieveIncludeActionWidget::~SieveIncludeActionWidget()
{

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
        //TODO
        //w->updateAddRemoveButton( addButtonEnabled, removeButtonEnabled );
    }
}

void SieveIncludeWidgetLister::generatedScript(QString &script, QStringList &requires)
{
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        SieveIncludeActionWidget *w = qobject_cast<SieveIncludeActionWidget*>( *wIt );
        //TOOD
        //w->generatedScript(script, requires);
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
