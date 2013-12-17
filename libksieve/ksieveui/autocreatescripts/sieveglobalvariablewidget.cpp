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
#include "sievescriptblockwidget.h"
#include "commonwidgets/sievehelpbutton.h"

#include <KPushButton>
#include <KLocalizedString>
#include <KLineEdit>

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QWhatsThis>
#include <QDebug>
#include <QDomNode>


namespace KSieveUi {
static int MINIMUMGLOBALVARIABLEACTION = 1;
static int MAXIMUMGLOBALVARIABLEACTION = 15;

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
    script += QLatin1String("global ");
    script += QString::fromLatin1("\"%1\";\n").arg(variableName);
    if (mSetValueTo->isChecked() && !mVariableValue->text().isEmpty()) {
        script += QString::fromLatin1("set \"%1\" \"%2\";\n").arg(variableName).arg(mVariableValue->text());
    }
}

void SieveGlobalVariableActionWidget::initWidget()
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins( 0, 0, 0, 0 );

    QLabel *lab = new QLabel(i18n("Variable name:"));
    mLayout->addWidget( lab, 1, 0 );

    mVariableName = new KLineEdit;
    mLayout->addWidget( mVariableName, 1, 1 );

    mSetValueTo = new QCheckBox(i18n("Set value to:"));
    mLayout->addWidget( mSetValueTo, 1, 2 );
    mSetValueTo->setChecked(false);

    mVariableValue = new KLineEdit;
    mVariableValue->setEnabled(false);
    mLayout->addWidget( mVariableValue, 1, 3 );

    connect(mSetValueTo, SIGNAL(clicked(bool)), mVariableValue, SLOT(setEnabled(bool)));

    mAdd = new KPushButton( this );
    mAdd->setIcon( KIcon( QLatin1String("list-add") ) );
    mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    mRemove = new KPushButton( this );
    mRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
    mRemove->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    mLayout->addWidget( mAdd, 1, 4 );
    mLayout->addWidget( mRemove, 1, 5 );

    connect( mAdd, SIGNAL(clicked()),
             this, SLOT(slotAddWidget()) );
    connect( mRemove, SIGNAL(clicked()),
             this, SLOT(slotRemoveWidget()) );
}

bool SieveGlobalVariableActionWidget::isInitialized() const
{
    return !mVariableName->text().isEmpty();
}

QString SieveGlobalVariableActionWidget::variableName() const
{
    return mVariableName->text();
}

void SieveGlobalVariableActionWidget::setVariableValue(const QString &name)
{
    mSetValueTo->setChecked(true);
    mVariableValue->setText(name);
    mVariableValue->setEnabled(true);
}

void SieveGlobalVariableActionWidget::loadScript(const QDomElement &element, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                mVariableName->setText(e.text());
            } else {
                error += i18n("Unknown tag \"%1\" during loading of variables.");
                qDebug()<<" SieveGlobalVariableActionWidget::loadScript unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
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
    SieveHelpButton *helpButton = new SieveHelpButton;
    lay->addWidget( helpButton );
    connect(helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

    mIncludeLister = new SieveGlobalVariableLister;
    lay->addWidget(mIncludeLister,0, Qt::AlignTop);
    setPageType(KSieveUi::SieveScriptBlockWidget::GlobalVariable);
    setLayout(lay);
}

SieveGlobalVariableWidget::~SieveGlobalVariableWidget()
{
}

void SieveGlobalVariableWidget::slotHelp()
{
    const QString help = i18n("A variable has global scope in all scripts that have declared it with the \"global\" command.  If a script uses that variable name without declaring it global, the name specifies a separate, non-global variable within that script.");
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

void SieveGlobalVariableWidget::loadScript(const QDomElement &element, QString &error)
{
    mIncludeLister->loadScript(element, error);
}

void SieveGlobalVariableWidget::loadSetVariable(const QDomElement &element, QString &error)
{
    mIncludeLister->loadSetVariable(element, error);
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

void SieveGlobalVariableLister::loadScript(const QDomElement &element, QString &error)
{
    SieveGlobalVariableActionWidget *w = static_cast<SieveGlobalVariableActionWidget *>(widgets().last());
    if (w->isInitialized()) {
        addWidgetAfterThisWidget(widgets().last());
        w = static_cast<SieveGlobalVariableActionWidget *>(widgets().last());
    }
    w->loadScript(element, error);
}

void SieveGlobalVariableLister::loadSetVariable(const QDomElement &element, QString &error)
{
    QString variableName;
    QString variableValue;
    int index = 0;
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("str")) {
                if (index == 0) {
                    variableName = e.text();
                } else if (index == 1) {
                    variableValue = e.text();
                } else {
                    qDebug()<<" SieveGlobalVariableLister::loadSetVariable too many argument:"<<index;
                }
                ++index;
            } else {
                qDebug()<<" SieveGlobalVariableLister::loadSetVariable unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }

    Q_FOREACH (QWidget *widget, widgets()) {
        SieveGlobalVariableActionWidget *w = static_cast<SieveGlobalVariableActionWidget *>(widget);
        if (w->variableName() == variableName) {
            w->setVariableValue(variableValue);
        }
    }
}

}

