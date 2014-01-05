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

#include "sieveforeverypartwidget.h"
#include "sievescriptblockwidget.h"
#include "autocreatescriptutil_p.h"
#include "commonwidgets/sievehelpbutton.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "editor/sieveeditorutil.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <KIcon>

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>
#include <QWhatsThis>
#include <QDomNode>
#include <QDebug>

namespace KSieveUi {
SieveForEveryPartWidget::SieveForEveryPartWidget(QWidget *parent)
    : SieveWidgetPageAbstract(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout;

    QWidget *w = new QWidget;
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    mHelpButton = new SieveHelpButton;
    topLayout->addWidget( mHelpButton );
    connect(mHelpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

    mForLoop = new QCheckBox(i18n("Add ForEveryPart loop"));
    topLayout->addWidget(mForLoop);


    QLabel *lab = new QLabel(i18n("Name (optional):"));
    lay->addWidget(lab);

    mName = new KLineEdit;
    mName->setEnabled(false);
    lay->addWidget(mName);

    topLayout->addWidget(w,0, Qt::AlignTop);

    connect(mForLoop, SIGNAL(clicked(bool)), mName, SLOT(setEnabled(bool)));
    setPageType(KSieveUi::SieveScriptBlockWidget::ForEveryPart);
    setLayout(topLayout);
}

SieveForEveryPartWidget::~SieveForEveryPartWidget()
{
}

void SieveForEveryPartWidget::slotHelp()
{
    const QString help = i18n("\"foreverypart\", which is an iterator that walks though every MIME part of a message, including nested parts, depth first, and applies the commands in the specified block to each of them.");
    const QString href = KSieveUi::SieveEditorUtil::helpUrl(KSieveUi::SieveEditorUtil::ForEveryPart);
    const QString fullWhatsThis = AutoCreateScriptUtil::createFullWhatsThis(help,href);
    QWhatsThis::showText( QCursor::pos(), fullWhatsThis, mHelpButton );
}

void SieveForEveryPartWidget::generatedScript(QString &script, QStringList &requires)
{
    if (mForLoop->isChecked()) {
        requires << QLatin1String("foreverypart");
        const QString loopName = mName->text();
        if (loopName.isEmpty()) {
            script += QLatin1String("foreverypart {");
        } else {
            script += QString::fromLatin1("foreverypart :name \"%1\" {").arg(loopName);
        }
    }
}

void SieveForEveryPartWidget::loadScript(const QDomElement &element, QString &error)
{
    QDomNode node = element.firstChild();
    QDomElement e = node.toElement();
    if (!e.isNull()) {
        const QString tagName = e.tagName();
        if (tagName == QLatin1String("tag")) {
            const QString tagValue = e.text();
            if (tagValue == QLatin1String("name")) {
                mName->setText(AutoCreateScriptUtil::strValue(e));
            } else {
                error += i18n("Unknown tagValue \"%1\" during loading loop \"for\"", tagValue);
                qDebug()<<" SieveForEveryPartWidget::loadScript unknown tagValue "<<tagValue;
            }
            mForLoop->setChecked(true);
            mName->setEnabled(true);
        } else {
            error += i18n("Unknown tag \"%1\" during loading loop \"for\"", tagName);
            qDebug()<<" SieveForEveryPartWidget::loadScript unknown tagName "<<tagName;
        }
    }
}
}

