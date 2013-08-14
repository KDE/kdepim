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

#include "sieveforeverypartwidget.h"
#include "sievescriptblockwidget.h"

#include <KLocale>
#include <KLineEdit>
#include <KIcon>

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>
#include <QWhatsThis>

namespace KSieveUi {
SieveForEveryPartWidget::SieveForEveryPartWidget(QWidget *parent)
    : SieveWidgetPageAbstract(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout;

    QWidget *w = new QWidget;
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QToolButton *helpButton = new QToolButton;
    helpButton->setToolTip(i18n("Help"));
    topLayout->addWidget( helpButton );
    helpButton->setIcon( KIcon( QLatin1String("help-hint") ) );
    connect(helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

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
    QWhatsThis::showText( QCursor::pos(), help );
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

}

#include "sieveforeverypartwidget.moc"
