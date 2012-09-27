/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
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

#include "kmcomposerautocorrectionwidget.h"
#include "kmcomposerautocorrection.h"
#include "ui_kmcomposerautocorrectionwidget.h"

#include "messagecomposersettings.h"
#include "selectspecialchar.h"

#include <KCharSelect>
#include <QTreeWidgetItem>

KMComposerAutoCorrectionWidget::KMComposerAutoCorrectionWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::KMComposerAutoCorrectionWidget),
  mAutoCorrection(0)
{
  ui->setupUi(this);

  ui->treeWidget->setSortingEnabled(true);
  ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);

  ui->add1->setEnabled(false);
  ui->add2->setEnabled(false);

  connect(ui->upperCase,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->upperUpper,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->ignoreDoubleSpace,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->autoReplaceNumber,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->capitalizeDaysName,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->advancedAutocorrection,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->enabledAutocorrection,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->typographicSingleQuotes, SIGNAL(clicked(bool)), this, SLOT(enableSingleQuotes(bool)));
  connect(ui->typographicDoubleQuotes, SIGNAL(clicked(bool)), this, SLOT(enableDoubleQuotes(bool)));
  connect(ui->singleQuote1, SIGNAL(clicked()), this, SLOT(selectSingleQuoteCharOpen()));
  connect(ui->singleQuote2, SIGNAL(clicked()), this, SLOT(selectSingleQuoteCharClose()));
  connect(ui->singleDefault, SIGNAL(clicked()), this, SLOT(setDefaultSingleQuotes()));
  connect(ui->doubleQuote1, SIGNAL(clicked()), this, SLOT(selectDoubleQuoteCharOpen()));
  connect(ui->doubleQuote2, SIGNAL(clicked()), this, SLOT(selectDoubleQuoteCharClose()));
  connect(ui->doubleDefault, SIGNAL(clicked()), this, SLOT(setDefaultDoubleQuotes()));
  connect(ui->advancedAutocorrection, SIGNAL(clicked(bool)), this, SLOT(enableAdvAutocorrection(bool)));
  connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addAutocorrectEntry()));
  connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeAutocorrectEntry()));
  connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(setFindReplaceText(QTreeWidgetItem*,int)));
  connect(ui->find, SIGNAL(textChanged(QString)), this, SLOT(enableAddRemoveButton()));
  connect(ui->replace, SIGNAL(textChanged(QString)), this, SLOT(enableAddRemoveButton()));
  connect(ui->abbreviation, SIGNAL(textChanged(QString)), this, SLOT(abbreviationChanged(QString)));
  connect(ui->twoUpperLetter, SIGNAL(textChanged(QString)), this, SLOT(twoUpperLetterChanged(QString)));
  connect(ui->add1, SIGNAL(clicked()), this, SLOT(addAbbreviationEntry()));
  connect(ui->remove1, SIGNAL(clicked()), this, SLOT(removeAbbreviationEntry()));
  connect(ui->add2, SIGNAL(clicked()), this, SLOT(addTwoUpperLetterEntry()));
  connect(ui->remove2, SIGNAL(clicked()), this, SLOT(removeTwoUpperLetterEntry()));
  connect(ui->typographicDoubleQuotes,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->typographicSingleQuotes,SIGNAL(clicked()),SIGNAL(changed()));
  connect(ui->abbreviationList,SIGNAL(itemClicked ( QListWidgetItem *)),SLOT(slotEnableDisableAbreviationList()));
  connect(ui->twoUpperLetterList,SIGNAL(itemClicked ( QListWidgetItem *)),SLOT(slotEnableDisableTwoUpperEntry()));
  slotEnableDisableAbreviationList();
  slotEnableDisableTwoUpperEntry();
}

KMComposerAutoCorrectionWidget::~KMComposerAutoCorrectionWidget()
{
  delete ui;
}

void KMComposerAutoCorrectionWidget::setAutoCorrection(KMComposerAutoCorrection * autoCorrect)
{
  mAutoCorrection = autoCorrect;
}

void KMComposerAutoCorrectionWidget::loadConfig()
{
    if(!mAutoCorrection)
      return;

    ui->enabledAutocorrection->setChecked(mAutoCorrection->isEnabledAutoCorrection());
    ui->upperCase->setChecked(mAutoCorrection->isUppercaseFirstCharOfSentence());
    ui->upperUpper->setChecked(mAutoCorrection->isFixTwoUppercaseChars());
    ui->ignoreDoubleSpace->setChecked(mAutoCorrection->isSingleSpaces());
    ui->autoReplaceNumber->setChecked(mAutoCorrection->isAutoFractions());
    ui->capitalizeDaysName->setChecked(mAutoCorrection->isCapitalizeWeekDays());
    ui->advancedAutocorrection->setChecked(mAutoCorrection->isAdvancedAutocorrect());

    /* tab 2 - Custom Quotes */
    ui->typographicDoubleQuotes->setChecked(mAutoCorrection->isReplaceDoubleQuotes());
    ui->typographicSingleQuotes->setChecked(mAutoCorrection->isReplaceSingleQuotes());
    m_singleQuotes = mAutoCorrection->typographicSingleQuotes();
    ui->singleQuote1->setText(m_singleQuotes.begin);
    ui->singleQuote2->setText(m_singleQuotes.end);
    m_doubleQuotes = mAutoCorrection->typographicDoubleQuotes();
    ui->doubleQuote1->setText(m_doubleQuotes.begin);
    ui->doubleQuote2->setText(m_doubleQuotes.end);
    enableSingleQuotes(ui->typographicSingleQuotes->isChecked());
    enableDoubleQuotes(ui->typographicDoubleQuotes->isChecked());

    /* tab 3 - Advanced Autocorrection */
    m_autocorrectEntries = mAutoCorrection->autocorrectEntries();
    ui->treeWidget->clear();
    QHash<QString, QString>::const_iterator i = m_autocorrectEntries.constBegin();
    QTreeWidgetItem * item = 0;
    while (i != m_autocorrectEntries.constEnd()) {
        item = new QTreeWidgetItem( ui->treeWidget, item );
        item->setText( 0, i.key() );
        item->setText( 1, i.value() );
        i++;
    }
    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);

    enableAdvAutocorrection(ui->advancedAutocorrection->isChecked());
    /* tab 4 - Exceptions */
    m_upperCaseExceptions = mAutoCorrection->upperCaseExceptions();
    m_twoUpperLetterExceptions = mAutoCorrection->twoUpperLetterExceptions();

    ui->twoUpperLetterList->clear();
    ui->twoUpperLetterList->addItems(m_twoUpperLetterExceptions.toList());

    ui->abbreviationList->clear();
    ui->abbreviationList->addItems(m_upperCaseExceptions.toList());
}

void KMComposerAutoCorrectionWidget::writeConfig()
{
  if(!mAutoCorrection)
    return;
  mAutoCorrection->setEnabledAutoCorrection(ui->enabledAutocorrection->isChecked());
  mAutoCorrection->setUppercaseFirstCharOfSentence(ui->upperCase->isChecked());
  mAutoCorrection->setFixTwoUppercaseChars(ui->upperUpper->isChecked());
  mAutoCorrection->setSingleSpaces(ui->ignoreDoubleSpace->isChecked());
  mAutoCorrection->setCapitalizeWeekDays(ui->capitalizeDaysName->isChecked());
  mAutoCorrection->setAdvancedAutocorrect(ui->advancedAutocorrection->isChecked());

  mAutoCorrection->setAutoFractions(ui->autoReplaceNumber->isChecked());

  mAutoCorrection->setAutocorrectEntries(m_autocorrectEntries);
  mAutoCorrection->setUpperCaseExceptions(m_upperCaseExceptions);
  mAutoCorrection->setTwoUpperLetterExceptions(m_twoUpperLetterExceptions);

  mAutoCorrection->setReplaceDoubleQuotes(ui->typographicDoubleQuotes->isChecked());
  mAutoCorrection->setReplaceSingleQuotes(ui->typographicSingleQuotes->isChecked());
  mAutoCorrection->setTypographicSingleQuotes(m_singleQuotes);
  mAutoCorrection->setTypographicDoubleQuotes(m_doubleQuotes);
  mAutoCorrection->writeConfig();

}

void KMComposerAutoCorrectionWidget::resetToDefault()
{
  ui->upperCase->setChecked(false);
  ui->upperUpper->setChecked(false);
  ui->ignoreDoubleSpace->setChecked(false);
  ui->capitalizeDaysName->setChecked(false);
  ui->advancedAutocorrection->setChecked(false);
  ui->typographicDoubleQuotes->setChecked(false);
  ui->typographicSingleQuotes->setChecked(false);
}

void KMComposerAutoCorrectionWidget::enableSingleQuotes(bool state)
{
  ui->singleQuote1->setEnabled(state);
  ui->singleQuote2->setEnabled(state);
  ui->singleDefault->setEnabled(state);
}

void KMComposerAutoCorrectionWidget::enableDoubleQuotes(bool state)
{
  ui->doubleQuote1->setEnabled(state);
  ui->doubleQuote2->setEnabled(state);
  ui->doubleDefault->setEnabled(state);
}

void KMComposerAutoCorrectionWidget::selectSingleQuoteCharOpen()
{
  SelectSpecialChar dlg(this);
  dlg.setCurrentChar(m_singleQuotes.begin);
  dlg.showSelectButton(false);
  if (dlg.exec()) {
    m_singleQuotes.begin = dlg.currentChar();
    ui->singleQuote1->setText(m_singleQuotes.begin);
    Q_EMIT changed();
  }
}

void KMComposerAutoCorrectionWidget::selectSingleQuoteCharClose()
{
  SelectSpecialChar dlg(this);
  dlg.showSelectButton(false);
  dlg.setCurrentChar(m_singleQuotes.end);
  if (dlg.exec()) {
    m_singleQuotes.end = dlg.currentChar();
    ui->singleQuote2->setText(m_singleQuotes.end);
    Q_EMIT changed();
  }
}

void KMComposerAutoCorrectionWidget::setDefaultSingleQuotes()
{
  m_singleQuotes = mAutoCorrection->typographicDefaultSingleQuotes();
  ui->singleQuote1->setText(m_singleQuotes.begin);
  ui->singleQuote2->setText(m_singleQuotes.end);
}

void KMComposerAutoCorrectionWidget::selectDoubleQuoteCharOpen()
{
  SelectSpecialChar dlg(this);
  dlg.showSelectButton(false);
  dlg.setCurrentChar(m_doubleQuotes.begin);
  if (dlg.exec()) {
    m_doubleQuotes.begin = dlg.currentChar();
    ui->doubleQuote1->setText(m_doubleQuotes.begin);
    Q_EMIT changed();
  }
}

void KMComposerAutoCorrectionWidget::selectDoubleQuoteCharClose()
{
  SelectSpecialChar dlg(this);
  dlg.showSelectButton(false);
  dlg.setCurrentChar(m_doubleQuotes.end);
  if (dlg.exec()) {
    m_doubleQuotes.end = dlg.currentChar();
    ui->doubleQuote2->setText(m_doubleQuotes.end);
    Q_EMIT changed();
  }
}

void KMComposerAutoCorrectionWidget::setDefaultDoubleQuotes()
{
  m_doubleQuotes = mAutoCorrection->typographicDefaultDoubleQuotes();
  ui->doubleQuote1->setText(m_doubleQuotes.begin);
  ui->doubleQuote2->setText(m_doubleQuotes.end);
}

void KMComposerAutoCorrectionWidget::enableAdvAutocorrection(bool state)
{
  ui->findLabel->setEnabled(state);
  ui->find->setEnabled(state);
  ui->replaceLabel->setEnabled(state);
  ui->replace->setEnabled(state);
  ui->addButton->setEnabled(state);
  ui->removeButton->setEnabled(state);
  ui->treeWidget->setEnabled(state);
}


void KMComposerAutoCorrectionWidget::addAutocorrectEntry()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem ();
    QString find = ui->find->text();
    bool modify = false;

    // Modify actually, not add, so we want to remove item from hash
    if (item && (find == item->text(0))) {
        m_autocorrectEntries.remove(find);
        modify = true;
    }

    m_autocorrectEntries.insert(find, ui->replace->text());
    ui->treeWidget->setSortingEnabled(false);
    if (modify) {
        item->setText(0,find);
        item->setText(1,ui->replace->text());
    } else {
        item = new QTreeWidgetItem( ui->treeWidget, item );
        item->setText( 0, find );
        item->setText( 1, ui->replace->text() );
    }

    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->setCurrentItem(item);
    Q_EMIT changed();
}

void KMComposerAutoCorrectionWidget::removeAutocorrectEntry()
{
  QTreeWidgetItem *item = ui->treeWidget->currentItem();
  if ( !item ) {
    return;
  }
  QTreeWidgetItem *below = ui->treeWidget->itemBelow( item );

  if ( below ) {
    kDebug() << "below";
    ui->treeWidget->setCurrentItem( below );
    delete item;
    item = 0;
  } else if ( ui->treeWidget->topLevelItemCount() > 0 ) {
    delete item;
    item = 0;
    ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem( ui->treeWidget->topLevelItemCount() - 1 )
    );
  }
  ui->treeWidget->setSortingEnabled(false);
  m_autocorrectEntries.remove(ui->find->text());
  Q_EMIT changed();
}

void KMComposerAutoCorrectionWidget::enableAddRemoveButton()
{
    QString find = ui->find->text();
    QString replace = ui->replace->text();
    int currentRow = -1;

    QTreeWidgetItem *item = 0;
    if (m_autocorrectEntries.contains(find)) {
        item = ui->treeWidget->findItems(find, Qt::MatchCaseSensitive).first();

    }
    bool enable = false;
    if ( !currentRow || find.isEmpty() || replace.isEmpty()) // disable if no text in find/replace
        enable = !(find.isEmpty() || replace.isEmpty());
    else if (item && find == item->text(0)) {
        // We disable add / remove button if no text for the replacement
        enable = !item->text(1).isEmpty();
        ui->addButton->setText(i18n("&Modify"));
    }
    else if (!item || !item->text(1).isEmpty()) {
        enable = true;
        ui->addButton->setText(i18n("&Add"));
    }

    if (item && replace == item->text(1))
        ui->addButton->setEnabled(false);
    else
        ui->addButton->setEnabled(enable);
    ui->removeButton->setEnabled(enable);

}

void KMComposerAutoCorrectionWidget::setFindReplaceText(QTreeWidgetItem*item ,int column)
{
  Q_UNUSED(column);
  ui->find->setText(item->text(0));
  ui->replace->setText(item->text(1));
  Q_EMIT changed();
}


void KMComposerAutoCorrectionWidget::abbreviationChanged(const QString &text)
{
  ui->add1->setEnabled(!text.isEmpty());
  Q_EMIT changed();
}

void KMComposerAutoCorrectionWidget::twoUpperLetterChanged(const QString &text)
{
  ui->add2->setEnabled(!text.isEmpty());
  Q_EMIT changed();
}

void KMComposerAutoCorrectionWidget::addAbbreviationEntry()
{
  const QString text = ui->abbreviation->text();
  if(text.isEmpty())
    return;
  if (!m_upperCaseExceptions.contains(text)) {
    m_upperCaseExceptions.insert(text);
    ui->abbreviationList->addItem(text);
  }
  ui->abbreviation->clear();
  slotEnableDisableAbreviationList();
  Q_EMIT changed();
}

void KMComposerAutoCorrectionWidget::removeAbbreviationEntry()
{
  const int currentRow = ui->abbreviationList->currentRow();
  QListWidgetItem *item = ui->abbreviationList->takeItem(currentRow);
  if(!item)
    return;
  m_upperCaseExceptions.remove(item->text());
  delete item;
  slotEnableDisableAbreviationList();
  Q_EMIT changed();
}

void KMComposerAutoCorrectionWidget::addTwoUpperLetterEntry()
{
  const QString text = ui->twoUpperLetter->text();
  if(text.isEmpty())
    return;
  if (!m_twoUpperLetterExceptions.contains(text)) {
    m_twoUpperLetterExceptions.insert(text);
    ui->twoUpperLetterList->addItem(text);
    Q_EMIT changed();
  }
  slotEnableDisableTwoUpperEntry();
  ui->twoUpperLetter->clear();

}

void KMComposerAutoCorrectionWidget::removeTwoUpperLetterEntry()
{
  const int currentRow = ui->twoUpperLetterList->currentRow();
  QListWidgetItem *item = ui->twoUpperLetterList->takeItem(currentRow);
  if(!item)
    return;
  m_twoUpperLetterExceptions.remove(item->text());
  delete item;
  slotEnableDisableTwoUpperEntry();
  Q_EMIT changed();
}


void KMComposerAutoCorrectionWidget::slotEnableDisableAbreviationList()
{
    const bool enable = (ui->abbreviationList->currentItem() != 0);
    ui->add1->setEnabled(enable && !ui->abbreviation->text().isEmpty());
    ui->remove1->setEnabled(enable);
}

void KMComposerAutoCorrectionWidget::slotEnableDisableTwoUpperEntry()
{
    const bool enable = (ui->twoUpperLetterList->currentItem() != 0);
    ui->add2->setEnabled(enable && !ui->twoUpperLetter->text().isEmpty());
    ui->remove2->setEnabled(enable);
}

#include "kmcomposerautocorrectionwidget.moc"
