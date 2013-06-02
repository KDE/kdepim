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

#include "sieveglobalvariablewidget.h"

#include <KPushButton>
#include <KLocale>
#include <KLineEdit>

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QWhatsThis>


namespace KSieveUi {
static int MINIMUMGLOBALVARIABLEACTION = 1;
static int MAXIMUMGLOBALVARIABLEACTION = 8;

SieveGlobalVariableActionWidget::SieveGlobalVariableActionWidget(QWidget *parent)
    : QWidget(parent)
{
    initWidget();
}

SieveGlobalVariableActionWidget::~SieveGlobalVariableActionWidget()
{

}

void SieveGlobalVariableActionWidget::generatedScript(QString &script)
{
    const QString variableName = mVariableName->text();
    if (variableName.isEmpty())
        return;
    script += QLatin1String("include ");
    //TODO
    script += QString::fromLatin1("\"%1\";\n").arg(variableName);
}

void SieveGlobalVariableActionWidget::initWidget()
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins( 0, 0, 0, 0 );

    QLabel *lab = new QLabel(i18n("Variable name:"));
    mLayout->addWidget( lab, 0, 0 );

    mVariableName = new KLineEdit;
    mLayout->addWidget( mVariableName, 0, 1 );

    mAdd = new KPushButton( this );
    mAdd->setIcon( KIcon( QLatin1String("list-add") ) );
    mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    mRemove = new KPushButton( this );
    mRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
    mRemove->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    mLayout->addWidget( mAdd, 1, 6 );
    mLayout->addWidget( mRemove, 1, 7 );

    connect( mAdd, SIGNAL(clicked()),
             this, SLOT(slotAddWidget()) );
    connect( mRemove, SIGNAL(clicked()),
             this, SLOT(slotRemoveWidget()) );
}

void SieveGlobalVariableActionWidget::slotAddWidget()
{
    emit addWidget( this );
}

void SieveGlobalVariableActionWidget::slotRemoveWidget()
{
    emit removeWidget( this );
}

void SieveGlobalVariableActionWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
    mAdd->setEnabled(addButtonEnabled);
    mRemove->setEnabled(removeButtonEnabled);
}

SieveGlobalVariableWidget::SieveGlobalVariableWidget(QWidget *parent)
    : SieveWidgetPageAbstract(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    QToolButton *helpButton = new QToolButton;
    helpButton->setToolTip(i18n("Help"));
    lay->addWidget( helpButton );
    helpButton->setIcon( KIcon( QLatin1String("help-hint") ) );
    connect(helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

    mIncludeLister = new SieveGlobalVariableLister;
    lay->addWidget(mIncludeLister,0, Qt::AlignTop);
    setLayout(lay);
}

SieveGlobalVariableWidget::~SieveGlobalVariableWidget()
{
}

void SieveGlobalVariableWidget::slotHelp()
{
    const QString help = i18n("The \"include\" command takes an optional \"location\" parameter, an optional \":once\" parameter, an optional \":optional\" parameter, and a single string argument representing the name of the script to include for processing at that point.");
    QWhatsThis::showText( QCursor::pos(), help );
}

void SieveGlobalVariableWidget::generatedScript(QString &script, QStringList &requires)
{
    QString result;
    QStringList lst;
    mIncludeLister->generatedScript(result, lst);
    if (!result.isEmpty()) {
        script += result;
        requires << lst;
    }
}

SieveGlobalVariableLister::SieveGlobalVariableLister(QWidget *parent)
    : KPIM::KWidgetLister(false, MINIMUMGLOBALVARIABLEACTION, MAXIMUMGLOBALVARIABLEACTION, parent)
{
    slotClear();
    updateAddRemoveButton();
}

SieveGlobalVariableLister::~SieveGlobalVariableLister()
{

}

void SieveGlobalVariableLister::slotAddWidget( QWidget *w )
{
    addWidgetAfterThisWidget( w );
    updateAddRemoveButton();
}

void SieveGlobalVariableLister::slotRemoveWidget( QWidget *w )
{
    removeWidget( w );
    updateAddRemoveButton();
}


void SieveGlobalVariableLister::updateAddRemoveButton()
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
        SieveGlobalVariableActionWidget *w = qobject_cast<SieveGlobalVariableActionWidget*>( *wIt );
        w->updateAddRemoveButton( addButtonEnabled, removeButtonEnabled );
    }
}

void SieveGlobalVariableLister::generatedScript(QString &script, QStringList &requires)
{
    requires << QLatin1String("include");
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        SieveGlobalVariableActionWidget *w = qobject_cast<SieveGlobalVariableActionWidget*>( *wIt );
        w->generatedScript(script);
    }
}

void SieveGlobalVariableLister::reconnectWidget(SieveGlobalVariableActionWidget *w )
{
    connect( w, SIGNAL(addWidget(QWidget*)),
             this, SLOT(slotAddWidget(QWidget*)), Qt::UniqueConnection );
    connect( w, SIGNAL(removeWidget(QWidget*)),
             this, SLOT(slotRemoveWidget(QWidget*)), Qt::UniqueConnection );
}

void SieveGlobalVariableLister::clearWidget( QWidget *aWidget )
{
    //TODO
}

QWidget *SieveGlobalVariableLister::createWidget( QWidget *parent )
{
    SieveGlobalVariableActionWidget *w = new SieveGlobalVariableActionWidget( parent);
    reconnectWidget( w );
    return w;
}

}

#include "sieveglobalvariablewidget.moc"
