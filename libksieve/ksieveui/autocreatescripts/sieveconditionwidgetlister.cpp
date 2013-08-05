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

#include "sieveconditionwidgetlister.h"
#include "autocreatescriptdialog.h"
#include "sieveeditorgraphicalmodewidget.h"
#include "sieveconditions/sieveconditionlist.h"
#include "sieveconditions/sievecondition.h"
#include "pimcommon/widgets/minimumcombobox.h"

#include <KPushButton>
#include <KDialog>
#include <KLocale>

#include <QGridLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QToolButton>
#include <QWhatsThis>
#include <QDebug>
#include <QDomElement>

using namespace KSieveUi;

static int MINIMUMCONDITION = 1;
static int MAXIMUMCONDITION = 8;

SieveConditionWidget::SieveConditionWidget(QWidget *parent)
    : QWidget(parent)
{
    initWidget();
}

SieveConditionWidget::~SieveConditionWidget()
{
    qDeleteAll(mConditionList);
    mConditionList.clear();
}

void SieveConditionWidget::setFilterCondition( QWidget *widget )
{
    if ( mLayout->itemAtPosition( 1, 2 ) ) {
        delete mLayout->itemAtPosition( 1, 2 )->widget();
    }

    if ( widget ) {
        mLayout->addWidget( widget, 1, 2 );
    } else {
        mLayout->addWidget( new QLabel( i18n( "Please select an condition." ), this ), 1, 2 );
    }
}

void SieveConditionWidget::generatedScript(QString &script, QStringList &requires)
{
    const int index = mComboBox->currentIndex();
    if (index != mComboBox->count()-1) {
        KSieveUi::SieveCondition *widgetCondition = mConditionList.at(mComboBox->currentIndex());
        QWidget *currentWidget = mLayout->itemAtPosition( 1, 2 )->widget();
        const QStringList lstRequires = widgetCondition->needRequires(currentWidget);
        Q_FOREACH (const QString &r, lstRequires) {
            if (!requires.contains(r)) {
                requires.append(r);
            }
        }
        script += mConditionList.at(mComboBox->currentIndex())->code(currentWidget) + QLatin1Char('\n');
    }
}

void SieveConditionWidget::initWidget()
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins( 0, 0, 0, 0 );

    mComboBox = new PimCommon::MinimumComboBox;
    mComboBox->setEditable( false );

    const QList<KSieveUi::SieveCondition*> list = KSieveUi::SieveConditionList::conditionList();
    QList<KSieveUi::SieveCondition*>::const_iterator it;
    QList<KSieveUi::SieveCondition*>::const_iterator end( list.constEnd() );
    int index = 0;
    for ( index = 0, it = list.constBegin(); it != end; ++it, ++index ) {        
        if ((*it)->needCheckIfServerHasCapability()) {
            if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains((*it)->serverNeedsCapability())) {
                // append to the list of actions:
                mConditionList.append( *it );

                // add (i18n-ized) name to combo box
                mComboBox->addItem( (*it)->label(),(*it)->name() );
            } else {
                delete (*it);
            }
        } else {
            // append to the list of actions:
            mConditionList.append( *it );

            // add (i18n-ized) name to combo box
            mComboBox->addItem( (*it)->label(),(*it)->name() );
        }
    }

    mHelpButton = new QToolButton;
    mHelpButton->setToolTip(i18n("Help"));
    mHelpButton->setEnabled(false);
    mLayout->addWidget( mHelpButton, 1, 0 );
    mHelpButton->setIcon( KIcon( QLatin1String("help-hint") ) );
    connect(mHelpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

    mComboBox->addItem(QLatin1String(""));
    mComboBox->setCurrentIndex(mComboBox->count()-1);
    mLayout->addWidget(mComboBox, 1, 1);
    connect( mComboBox, SIGNAL(activated(int)),
             this, SLOT(slotConditionChanged(int)) );

    mComboBox->setMaxCount( mComboBox->count() );
    mComboBox->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    mComboBox->adjustSize();

    mAdd = new KPushButton( this );
    mAdd->setIcon( KIcon( QLatin1String("list-add") ) );
    mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    mRemove = new KPushButton( this );
    mRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
    mRemove->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    mLayout->addWidget( mAdd, 1, 3 );
    mLayout->addWidget( mRemove, 1, 4 );

    // redirect focus to the filter action combo box
    setFocusProxy( mComboBox );

    connect( mAdd, SIGNAL(clicked()),
             this, SLOT(slotAddWidget()) );
    connect( mRemove, SIGNAL(clicked()),
             this, SLOT(slotRemoveWidget()) );
    setFilterCondition(0);
}

void SieveConditionWidget::slotHelp()
{
    const int index = mComboBox->currentIndex();
    if (index < mConditionList.count()) {
        KSieveUi::SieveCondition* condition = mConditionList.at( index );
        const QString help = condition->help();
        QWhatsThis::showText( QCursor::pos(), help );
    }
}

void SieveConditionWidget::slotConditionChanged(int index)
{
    if (index < mConditionList.count()) {
        KSieveUi::SieveCondition* condition = mConditionList.at( index );
        mHelpButton->setEnabled(!condition->help().isEmpty());
        setFilterCondition( condition->createParamWidget(this) );
    } else {
        setFilterCondition( 0 );
        mHelpButton->setEnabled(false);
    }
}

void SieveConditionWidget::slotAddWidget()
{
    emit addWidget( this );
}

void SieveConditionWidget::slotRemoveWidget()
{
    emit removeWidget( this );
}

void SieveConditionWidget::reset()
{
    //TODO
}

void SieveConditionWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
    mAdd->setEnabled(addButtonEnabled);
    mRemove->setEnabled(removeButtonEnabled);
}

void SieveConditionWidget::setCondition(const QString &conditionName, const QDomElement &element, bool notCondition)
{
    const int index = mComboBox->findData(conditionName);
    if (index != -1) {
        mComboBox->setCurrentIndex(index);
        slotConditionChanged(index);
        KSieveUi::SieveCondition* condition = mConditionList.at( index );
        condition->setParamWidgetValue(element, this, notCondition);
    }
}

SieveConditionWidgetLister::SieveConditionWidgetLister(QWidget *parent)
    : KPIM::KWidgetLister(false, MINIMUMCONDITION, MAXIMUMCONDITION, parent)
{
    slotClear();
    updateAddRemoveButton();
}

SieveConditionWidgetLister::~SieveConditionWidgetLister()
{

}

void SieveConditionWidgetLister::slotAddWidget( QWidget *w )
{
    addWidgetAfterThisWidget( w );
    updateAddRemoveButton();
}

void SieveConditionWidgetLister::slotRemoveWidget( QWidget *w )
{
    removeWidget( w );
    updateAddRemoveButton();
}


void SieveConditionWidgetLister::updateAddRemoveButton()
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
        SieveConditionWidget *w = qobject_cast<SieveConditionWidget*>( *wIt );
        w->updateAddRemoveButton( addButtonEnabled, removeButtonEnabled );
    }
}

void SieveConditionWidgetLister::reconnectWidget( SieveConditionWidget *w )
{
    connect( w, SIGNAL(addWidget(QWidget*)),
             this, SLOT(slotAddWidget(QWidget*)), Qt::UniqueConnection );
    connect( w, SIGNAL(removeWidget(QWidget*)),
             this, SLOT(slotRemoveWidget(QWidget*)), Qt::UniqueConnection );
}

void SieveConditionWidgetLister::clearWidget( QWidget *aWidget )
{
    //TODO
}

QWidget *SieveConditionWidgetLister::createWidget( QWidget *parent )
{
    SieveConditionWidget *w = new SieveConditionWidget(parent);
    reconnectWidget( w );
    return w;
}

void SieveConditionWidgetLister::generatedScript(QString &script, int &numberOfCondition, QStringList &requires)
{
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    bool wasFirst = true;
    for ( ; wIt != wEnd ;++wIt ) {
        QString condition;
        SieveConditionWidget *w = qobject_cast<SieveConditionWidget*>( *wIt );
        w->generatedScript(condition, requires);
        if (!condition.isEmpty()) {
            if (!wasFirst) {
                script += QLatin1String(", ");
            }
            script += condition;
            wasFirst = false;
            ++numberOfCondition;
        }
    }
}

int SieveConditionWidgetLister::conditionNumber() const
{
    return widgets().count();
}

void SieveConditionWidgetLister::loadTest(const QDomElement &element, bool notCondition)
{
    QDomElement testElement = element;
    if (notCondition) {
        QDomNode node = element.firstChild();
        if (!node.isNull()) {
            testElement = node.toElement();
        }
    }
    if (testElement.hasAttribute(QLatin1String("name"))) {
        const QString conditionName = testElement.attribute(QLatin1String("name"));
        SieveConditionWidget *w = qobject_cast<SieveConditionWidget*>( widgets().last() );
        w->setCondition(conditionName, testElement, notCondition);
    }
}

void SieveConditionWidgetLister::loadScript(const QDomElement &e, bool uniqTest, bool notCondition)
{
    if (uniqTest) {
        loadTest(e, notCondition);
    } else {
        bool firstCondition = true;
        QDomElement element = e;
        if (notCondition) {
            QDomNode node = e.firstChild();
            if (!node.isNull()) {
                element = node.toElement();
            }
        }
        QDomNode node = element.firstChild();
        while (!node.isNull()) {
            QDomElement e = node.toElement();
            if (!e.isNull()) {
                const QString tagName = e.tagName();
                if (tagName == QLatin1String("testlist")) {
                    QDomNode testNode = e.firstChild();
                    while (!testNode.isNull()) {
                        QDomElement testElement = testNode.toElement();
                        if (!testElement.isNull()) {
                            const QString testTagName = testElement.tagName();
                            if (testTagName == QLatin1String("test")) {
                                if (testElement.hasAttribute(QLatin1String("name"))) {
                                    QString conditionName = testElement.attribute(QLatin1String("name"));
                                    if (firstCondition) {
                                        firstCondition = false;
                                    } else {
                                        addWidgetAfterThisWidget(widgets().last());
                                    }
                                    SieveConditionWidget *w = qobject_cast<SieveConditionWidget*>( widgets().last() );
                                    if (conditionName == QLatin1String("not")) {
                                        notCondition = true;
                                        QDomNode notNode = testElement.firstChild();
                                        QDomElement notElement = notNode.toElement();
                                        if (notElement.hasAttribute(QLatin1String("name"))) {
                                            conditionName = notElement.attribute(QLatin1String("name"));
                                        }
                                        w->setCondition(conditionName, notElement, notCondition);
                                    } else {
                                        notCondition = false;
                                        w->setCondition(conditionName, testElement, notCondition);
                                    }
                                }
                            } else {
                                qDebug()<<" unknown condition tag: "<<testTagName;
                            }
                        }
                        testNode = testNode.nextSibling();
                    }
                }
            }
            node = node.nextSibling();
        }
    }
}

#include "sieveconditionwidgetlister.moc"
