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

#include "sieveactionwidgetlister.h"
#include "autocreatescriptdialog.h"
#include "sieveeditorgraphicalmodewidget.h"
#include "sievescriptdescriptiondialog.h"
#include "sieveactions/sieveaction.h"
#include "sieveactions/sieveactionlist.h"
#include "pimcommon/widgets/minimumcombobox.h"

#include <KPushButton>
#include <KDialog>
#include <KLocale>

#include <QGridLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QWhatsThis>

using namespace KSieveUi;

static int MINIMUMACTION = 1;
static int MAXIMUMACTION = 8;
static QString INDENTACTION = QLatin1String("    ");


SieveActionWidget::SieveActionWidget(QWidget *parent)
    : QWidget(parent)
{
    initWidget();
}

SieveActionWidget::~SieveActionWidget()
{
    qDeleteAll(mActionList);
    mActionList.clear();
}

void SieveActionWidget::setFilterAction( QWidget *widget )
{
    if ( mLayout->itemAtPosition( 1, 3 ) ) {
        delete mLayout->itemAtPosition( 1, 3 )->widget();
    }

    if ( widget ) {
        mLayout->addWidget( widget, 1, 3 );
    } else {
        mLayout->addWidget( new QLabel( i18n( "Please select an action." ), this ), 1, 3 );
    }
}

void SieveActionWidget::generatedScript(QString &script, QStringList &requires)
{
    const int index = mComboBox->currentIndex();
    if (index != mComboBox->count()-1) {
        KSieveUi::SieveAction *widgetAction = mActionList.at(mComboBox->currentIndex());
        QWidget *currentWidget = mLayout->itemAtPosition( 1, 3 )->widget();
        const QStringList lstRequires = widgetAction->needRequires(currentWidget);
        Q_FOREACH (const QString &r, lstRequires) {
            if (!requires.contains(r)) {
                requires.append(r);
            }
        }
        QString comment = widgetAction->comment();
        if (!comment.isEmpty()) {
            script += QLatin1Char('#') + comment.replace(QLatin1Char('\n'), QLatin1String("\n#")) + QLatin1Char('\n');
        }
        script += INDENTACTION + widgetAction->code(currentWidget) + QLatin1Char('\n');
    }
}

void SieveActionWidget::initWidget()
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins( 0, 0, 0, 0 );

    mComboBox = new PimCommon::MinimumComboBox;
    mComboBox->setEditable( false );
    const QList<KSieveUi::SieveAction*> list = KSieveUi::SieveActionList::actionList();
    QList<KSieveUi::SieveAction*>::const_iterator it;
    QList<KSieveUi::SieveAction*>::const_iterator end( list.constEnd() );
    int index = 0;
    for ( index = 0, it = list.constBegin(); it != end; ++it, ++index ) {
        if ((*it)->needCheckIfServerHasCapability()) {
            if (SieveEditorGraphicalModeWidget::sieveCapabilities().contains((*it)->serverNeedsCapability())) {
                // append to the list of actions:
                mActionList.append( *it );

                // add (i18n-ized) name to combo box
                mComboBox->addItem( (*it)->label(),(*it)->name() );
            } else {
                delete (*it);
            }
        } else {
            // append to the list of actions:
            mActionList.append( *it );

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

    mCommentButton = new QToolButton;
    mCommentButton->setToolTip(i18n("Add comment"));
    mCommentButton->setEnabled(false);
    mLayout->addWidget( mCommentButton, 1, 1 );
    mCommentButton->setIcon( KIcon( QLatin1String("view-pim-notes") ) );
    connect(mCommentButton, SIGNAL(clicked()), this, SLOT(slotAddComment()));


    mComboBox->addItem(QLatin1String(""));
    mComboBox->setCurrentIndex(mComboBox->count()-1);

    mComboBox->setMaxCount( mComboBox->count() );
    mComboBox->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    mComboBox->adjustSize();
    mLayout->addWidget(mComboBox, 1, 2);

    updateGeometry();

    connect( mComboBox, SIGNAL(activated(int)),
             this, SLOT(slotActionChanged(int)) );


    mAdd = new KPushButton( this );
    mAdd->setIcon( KIcon( QLatin1String("list-add") ) );
    mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    mRemove = new KPushButton( this );
    mRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
    mRemove->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    mLayout->addWidget( mAdd, 1, 4 );
    mLayout->addWidget( mRemove, 1, 5 );

    // redirect focus to the filter action combo box
    setFocusProxy( mComboBox );

    connect( mAdd, SIGNAL(clicked()),
             this, SLOT(slotAddWidget()) );
    connect( mRemove, SIGNAL(clicked()),
             this, SLOT(slotRemoveWidget()) );
    setFilterAction(0);
}


void SieveActionWidget::slotHelp()
{
    const int index = mComboBox->currentIndex();
    if (index < mActionList.count()) {
        KSieveUi::SieveAction* action = mActionList.at( index );
        const QString help = action->help();
        QWhatsThis::showText( QCursor::pos(), help );
    }
}

void SieveActionWidget::slotAddComment()
{
    const int index = mComboBox->currentIndex();
    if (index < mActionList.count()) {
        KSieveUi::SieveAction* action = mActionList.at( index );
        const QString comment = action->comment();
        QPointer<SieveScriptDescriptionDialog> dlg = new SieveScriptDescriptionDialog;
        dlg->setDescription(comment);
        if (dlg->exec()) {
            action->setComment(dlg->description());
        }
        delete dlg;
    }
}

void SieveActionWidget::slotActionChanged(int index)
{
    if (index < mActionList.count()) {
        KSieveUi::SieveAction* action = mActionList.at( index );
        mHelpButton->setEnabled(!action->help().isEmpty());
        mCommentButton->setEnabled(true);
        setFilterAction( action->createParamWidget(this) );
        //All actions after stop will not execute => don't allow to add more actions.
        const bool enableAddAction = (action->name() != QLatin1String("stop"));
        mAdd->setEnabled(enableAddAction);

    } else {
        mAdd->setEnabled(true);
        mCommentButton->setEnabled(false);
        setFilterAction( 0 );
        mHelpButton->setEnabled(false);
    }
}

void SieveActionWidget::slotAddWidget()
{
    emit addWidget( this );
}

void SieveActionWidget::slotRemoveWidget()
{
    emit removeWidget( this );
}

void SieveActionWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
    mAdd->setEnabled(addButtonEnabled);
    mRemove->setEnabled(removeButtonEnabled);
}

SieveActionWidgetLister::SieveActionWidgetLister(QWidget *parent)
    : KPIM::KWidgetLister(false, MINIMUMACTION, MAXIMUMACTION, parent)
{
    slotClear();
    updateAddRemoveButton();
}

SieveActionWidgetLister::~SieveActionWidgetLister()
{

}

void SieveActionWidgetLister::slotAddWidget( QWidget *w )
{
    addWidgetAfterThisWidget( w );
    updateAddRemoveButton();
}

void SieveActionWidgetLister::slotRemoveWidget( QWidget *w )
{
    removeWidget( w );
    updateAddRemoveButton();
}


void SieveActionWidgetLister::updateAddRemoveButton()
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
        SieveActionWidget *w = qobject_cast<SieveActionWidget*>( *wIt );
        w->updateAddRemoveButton( addButtonEnabled, removeButtonEnabled );
    }
}

void SieveActionWidgetLister::generatedScript(QString &script, QStringList &requires)
{
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        SieveActionWidget *w = qobject_cast<SieveActionWidget*>( *wIt );
        w->generatedScript(script, requires);
    }
}

void SieveActionWidgetLister::reconnectWidget( SieveActionWidget *w )
{
    connect( w, SIGNAL(addWidget(QWidget*)),
             this, SLOT(slotAddWidget(QWidget*)), Qt::UniqueConnection );
    connect( w, SIGNAL(removeWidget(QWidget*)),
             this, SLOT(slotRemoveWidget(QWidget*)), Qt::UniqueConnection );
}

void SieveActionWidgetLister::clearWidget( QWidget *aWidget )
{
    //TODO
}

QWidget *SieveActionWidgetLister::createWidget( QWidget *parent )
{
    SieveActionWidget *w = new SieveActionWidget( parent);
    reconnectWidget( w );
    return w;
}

int SieveActionWidgetLister::actionNumber() const
{
    return widgets().count();
}


#include "sieveactionwidgetlister.moc"
