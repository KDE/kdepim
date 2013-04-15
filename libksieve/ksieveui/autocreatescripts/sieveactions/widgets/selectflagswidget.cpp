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

#include "selectflagswidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLineEdit>
#include <KLocale>

#include <QPushButton>
#include <QHBoxLayout>


using namespace KSieveUi;

SelectFlagsListDialog::SelectFlagsListDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Flags" ) );
    setButtons( Ok|Cancel );
    setButtonFocus( Ok );
    mListWidget = new SelectFlagsListWidget;
    setMainWidget(mListWidget);
}

SelectFlagsListDialog::~SelectFlagsListDialog()
{
}

void SelectFlagsListDialog::setFlags(const QStringList& list)
{
    mListWidget->setFlags(list);
}

QStringList SelectFlagsListDialog::flags() const
{
    return mListWidget->flags();
}

SelectFlagsListWidget::SelectFlagsListWidget(QWidget *parent)
    : QListWidget(parent)
{
    init();
}

SelectFlagsListWidget::~SelectFlagsListWidget()
{
}

void SelectFlagsListWidget::init()
{
    QListWidgetItem *item = new QListWidgetItem(QLatin1String("\\\\Deleted"), this);
    item->setCheckState(Qt::Unchecked);
    item = new QListWidgetItem(QLatin1String("\\\\Answered"), this);
    item->setCheckState(Qt::Unchecked);
    item = new QListWidgetItem(QLatin1String("\\\\Flagged"), this);
    item->setCheckState(Qt::Unchecked);
    //TODO
}

void SelectFlagsListWidget::setFlags(const QStringList& list)
{
    const int numberOfItem = count();
    for (int i = 0; i < numberOfItem; ++i) {
        QListWidgetItem *it = item(i);
        if (list.contains(it->text())) {
            it->setCheckState(Qt::Checked);
        }
    }
}

QStringList SelectFlagsListWidget::flags() const
{
    QStringList result;
    const int numberOfItem = count();
    for (int i = 0; i < numberOfItem; ++i) {
        QListWidgetItem *it = item(i);
        if (it->checkState() == Qt::Checked) {
            result<<it->text();
        }
    }
    return result;
}

SelectFlagsWidget::SelectFlagsWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mEdit = new KLineEdit;
    mEdit->setReadOnly(true);
    lay->addWidget(mEdit);
    QPushButton *selectFlags = new QPushButton(i18n("..."));
    connect(selectFlags, SIGNAL(clicked(bool)), this, SLOT(slotSelectFlags()));
    lay->addWidget(selectFlags);
    setLayout(lay);
}

SelectFlagsWidget::~SelectFlagsWidget()
{
}

void SelectFlagsWidget::slotSelectFlags()
{
    QPointer<SelectFlagsListDialog> dialog = new SelectFlagsListDialog(this);
    dialog->setFlags(AutoCreateScriptUtil::createListFromString(mEdit->text()));
    if (dialog->exec()) {
        const QStringList lstFlags = dialog->flags();
        mCode.clear();
        if (!lstFlags.isEmpty()) {
            mCode = AutoCreateScriptUtil::createList(lstFlags);
        }
    }
    mEdit->setText(mCode);
    delete dialog;
}

QString SelectFlagsWidget::code() const
{
    return mCode;
}

#include "selectflagswidget.moc"
