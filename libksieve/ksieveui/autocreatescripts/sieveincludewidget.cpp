/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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
#include "sievescriptblockwidget.h"
#include "autocreatescriptutil_p.h"
#include "commonwidgets/sievehelpbutton.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "editor/sieveeditorutil.h"

#include <KPushButton>
#include <KLocalizedString>
#include <KLineEdit>

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QWhatsThis>
#include <QDomNode>
#include <QDebug>

namespace KSieveUi {
static int MINIMUMINCLUDEACTION = 1;
static int MAXIMUMINCLUDEACTION = 20;

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
    addItem(i18n("personal"), QLatin1String(":personal"));
    addItem(i18n("global"), QLatin1String(":global"));
}

QString SieveIncludeLocation::code() const
{
    return itemData(currentIndex()).toString();
}

void SieveIncludeLocation::setCode(const QString &code, QString &error)
{
    const int index = findData(code);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        error += i18n("Unknown location type \"%1\" during parsing includes", code);
        setCurrentIndex(0);
    }
}

SieveIncludeActionWidget::SieveIncludeActionWidget(QWidget *parent)
    : QWidget(parent)
{
    initWidget();
}

SieveIncludeActionWidget::~SieveIncludeActionWidget()
{
}

void SieveIncludeActionWidget::loadScript(const QDomElement &element, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (tagValue == QLatin1String("personal") ||
                        tagValue == QLatin1String("global")) {
                    mLocation->setCode(AutoCreateScriptUtil::tagValue(tagValue), error);
                } else if (tagValue == QLatin1String("optional")) {
                    mOptional->setChecked(true);
                } else if (tagValue == QLatin1String("once")) {
                    mOnce->setChecked(true);
                } else {
                    qDebug()<<" SieveIncludeActionWidget::loadScript unknown tagValue "<<tagValue;
                }
            } else if (tagName == QLatin1String("str")) {
                mIncludeName->setText(e.text());
            } else {
                qDebug()<<" SieveIncludeActionWidget::loadScript unknown tagName "<<tagName;
            }
        }
        node = node.nextSibling();
    }
}

void SieveIncludeActionWidget::generatedScript(QString &script)
{
    const QString includeName = mIncludeName->text();
    if (includeName.isEmpty())
        return;
    script += QLatin1String("include ");
    script += mLocation->code() + QLatin1Char(' ');
    if (mOptional->isChecked()) {
        script += QLatin1String(":optional ");
    }
    if (mOnce->isChecked()) {
        script += QLatin1String(":once ");
    }
    script += QString::fromLatin1("\"%1\";\n").arg(includeName);
}

void SieveIncludeActionWidget::initWidget()
{
    mLayout = new QGridLayout(this);
    mLayout->setContentsMargins( 0, 0, 0, 0 );

    QLabel *lab = new QLabel(i18n("Include:"));
    mLayout->addWidget( lab, 1, 0 );
    mLocation = new SieveIncludeLocation;
    mLayout->addWidget( mLocation, 1, 1 );

    lab = new QLabel(i18n("Name:"));
    mLayout->addWidget( lab, 1, 2 );

    mIncludeName = new KLineEdit;
    mLayout->addWidget( mIncludeName, 1, 3 );

    mOptional = new QCheckBox(i18n("Optional"));
    mLayout->addWidget( mOptional, 1, 4 );

    mOnce = new QCheckBox(i18n("Once"));
    mLayout->addWidget( mOnce, 1, 5 );

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

void SieveIncludeActionWidget::slotAddWidget()
{
    emit addWidget( this );
}

void SieveIncludeActionWidget::slotRemoveWidget()
{
    emit removeWidget( this );
}

bool SieveIncludeActionWidget::isInitialized() const
{
    return !mIncludeName->text().isEmpty();
}

void SieveIncludeActionWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
    mAdd->setEnabled(addButtonEnabled);
    mRemove->setEnabled(removeButtonEnabled);
}

SieveIncludeWidget::SieveIncludeWidget(QWidget *parent)
    : SieveWidgetPageAbstract(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mHelpButton = new SieveHelpButton;
    lay->addWidget( mHelpButton );
    connect(mHelpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

    mIncludeLister = new SieveIncludeWidgetLister;
    lay->addWidget(mIncludeLister,0, Qt::AlignTop);
    setPageType(KSieveUi::SieveScriptBlockWidget::Include);
    setLayout(lay);
}

SieveIncludeWidget::~SieveIncludeWidget()
{
}

void SieveIncludeWidget::slotHelp()
{
    const QString help = i18n("The \"include\" command takes an optional \"location\" parameter, an optional \":once\" parameter, an optional \":optional\" parameter, and a single string argument representing the name of the script to include for processing at that point.");
    const QString href = QLatin1String("http://tools.ietf.org/html/rfc6609#page-4");
    const QString fullWhatsThis = AutoCreateScriptUtil::createFullWhatsThis(help,href);
    QWhatsThis::showText( QCursor::pos(), fullWhatsThis, mHelpButton );
}

void SieveIncludeWidget::generatedScript(QString &script, QStringList &requires)
{
    QString result;
    QStringList lst;
    mIncludeLister->generatedScript(result, lst);
    if (!result.isEmpty()) {
        script += result;
        requires << lst;
    }
}

void SieveIncludeWidget::loadScript(const QDomElement &element, QString &error)
{
    mIncludeLister->loadScript(element, error);
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
    requires << QLatin1String("include");
    const QList<QWidget*> widgetList = widgets();
    QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
    QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
    for ( ; wIt != wEnd ;++wIt ) {
        SieveIncludeActionWidget *w = qobject_cast<SieveIncludeActionWidget*>( *wIt );
        w->generatedScript(script);
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

void SieveIncludeWidgetLister::loadScript(const QDomElement &element, QString &error)
{
    if (widgets().count() == MAXIMUMINCLUDEACTION) {
        error += QLatin1Char('\n') + i18n("We can not add more includes elements.") + QLatin1Char('\n');
        return;
    }
    SieveIncludeActionWidget *w = static_cast<SieveIncludeActionWidget *>(widgets().last());
    if (w->isInitialized()) {
        addWidgetAfterThisWidget(widgets().last());
        w = static_cast<SieveIncludeActionWidget *>(widgets().last());
    }
    w->loadScript(element, error);
}

}

