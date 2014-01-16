/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "commonwidgets/sievehelpbutton.h"
#include "autocreatescriptutil_p.h"
#include "pimcommon/widgets/minimumcombobox.h"

#include <KPushButton>
#include <KLocalizedString>

#include <QGridLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QWhatsThis>
#include <QDomElement>

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

bool SieveActionWidget::isConfigurated() const
{
    return (mComboBox->currentIndex() != (mComboBox->count()-1));
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

void SieveActionWidget::generatedScript(QString &script, QStringList &requires, bool onlyActions)
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
        script += (onlyActions ? QString() : INDENTACTION) + widgetAction->code(currentWidget) + QLatin1Char('\n');
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
    QStringList listCapabilities = SieveEditorGraphicalModeWidget::sieveCapabilities();
    //imapflags was old name of imap4flags but still used.
    if (listCapabilities.contains(QLatin1String("imap4flags"))) {
        listCapabilities.append(QLatin1String("imapflags"));
    }
    for ( index = 0, it = list.constBegin(); it != end; ++it, ++index ) {
        if ((*it)->needCheckIfServerHasCapability()) {
            if (listCapabilities.contains((*it)->serverNeedsCapability())) {
                // append to the list of actions:
                mActionList.append( *it );
                connect(*it, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
                // add (i18n-ized) name to combo box
                mComboBox->addItem( (*it)->label(),(*it)->name() );
            } else {
                delete (*it);
            }
        } else {
            // append to the list of actions:
            mActionList.append( *it );
            connect(*it, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
            // add (i18n-ized) name to combo box
            mComboBox->addItem( (*it)->label(),(*it)->name() );
        }
    }

    mHelpButton = new SieveHelpButton;
    mHelpButton->setEnabled(false);
    connect(mHelpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));
    mLayout->addWidget( mHelpButton, 1, 0 );

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
        const QString href = action->href();
        const QString fullWhatsThis = AutoCreateScriptUtil::createFullWhatsThis(help,href);
        QWhatsThis::showText( QCursor::pos(), fullWhatsThis, mHelpButton );
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
    Q_EMIT valueChanged();
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

bool SieveActionWidget::setAction(const QString &actionName, const QDomElement &element, const QString &comment, QString &error)
{
    const int index = mComboBox->findData(actionName);
    bool result = false;
    if (index != -1) {
        mComboBox->setCurrentIndex(index);
        slotActionChanged(index);
        KSieveUi::SieveAction* action = mActionList.at( index );
        result = action->setParamWidgetValue(element, this, error);
        action->setComment(comment);
    }
    return result;
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

void SieveActionWidgetLister::generatedScript(QString &script, QStringList &requires, bool onlyActions)
{
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        SieveActionWidget *w = qobject_cast<SieveActionWidget*>( *wIt );
        w->generatedScript(script, requires, onlyActions);
    }
}

void SieveActionWidgetLister::reconnectWidget( SieveActionWidget *w )
{
    connect( w, SIGNAL(addWidget(QWidget*)),
             this, SLOT(slotAddWidget(QWidget*)), Qt::UniqueConnection );
    connect( w, SIGNAL(removeWidget(QWidget*)),
             this, SLOT(slotRemoveWidget(QWidget*)), Qt::UniqueConnection );
    connect( w, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()), Qt::UniqueConnection);
}

void SieveActionWidgetLister::clearWidget( QWidget *aWidget )
{
    Q_UNUSED(aWidget);
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

void SieveActionWidgetLister::loadScript(const QDomElement &element, bool onlyActions, QString &error)
{
    QString comment;
    if (onlyActions) {
        if (!element.isNull()) {
            const QString tagName = element.tagName();
            if (tagName == QLatin1String("action")) {
                if (element.hasAttribute(QLatin1String("name"))) {
                    const QString actionName = element.attribute(QLatin1String("name"));
                    SieveActionWidget *w = qobject_cast<SieveActionWidget*>( widgets().last() );
                    if (w->isConfigurated()) {
                        addWidgetAfterThisWidget(widgets().last());
                        w = qobject_cast<SieveActionWidget*>( widgets().last() );
                    }
                    w->setAction(actionName, element, comment, error);
                    //comment.clear();
                } else if (tagName == QLatin1String("crlf")) {
                    //nothing
                } else {
                    qDebug()<<" SieveActionWidgetLister::loadScript don't have name attribute "<<tagName;
                }
            }
        }
    } else {
        bool firstAction = true;
        QDomNode node = element.firstChild();
        while (!node.isNull()) {
            QDomElement e = node.toElement();
            if (!e.isNull()) {
                const QString tagName = e.tagName();
                if (tagName == QLatin1String("action") || tagName == QLatin1String("control")/*for break action*/) {
                    if (e.hasAttribute(QLatin1String("name"))) {
                        const QString actionName = e.attribute(QLatin1String("name"));
                        if (tagName == QLatin1String("control") && actionName == QLatin1String("if")) {
                            qDebug()<<"We found an loop if in a loop if. Not supported";
                            error += QLatin1Char('\n') + i18n("We detected a loop if in a loop if. It's not supported") + QLatin1Char('\n');
                        }
                        if (firstAction) {
                            firstAction = false;
                        } else {
                            addWidgetAfterThisWidget(widgets().last());
                        }
                        SieveActionWidget *w = qobject_cast<SieveActionWidget*>( widgets().last() );
                        w->setAction(actionName, e, comment, error);
                        comment.clear();
                    } else {
                        qDebug()<<" SieveActionWidgetLister::loadScript don't have name attribute "<<tagName;
                    }
                } else if (tagName == QLatin1String("comment")) {
                    if (!comment.isEmpty()) {
                        comment += QLatin1Char('\n');
                    }
                    comment += e.text();
                } else if (tagName == QLatin1String("crlf")) {
                    //nothing
                } else {
                    qDebug()<<" SieveActionWidgetLister::loadScript unknown tagName "<<tagName;
                }
            }
            node = node.nextSibling();
        }
    }
}



