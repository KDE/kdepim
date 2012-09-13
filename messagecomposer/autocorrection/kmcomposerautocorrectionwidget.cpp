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

#include <KCharSelect>


KMComposerAutoCorrectionWidget::KMComposerAutoCorrectionWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::KMComposerAutoCorrectionWidget),
  mAutoCorrection(0)
{
  ui->setupUi(this);

  ui->tableWidget->setSortingEnabled(true);
  ui->tableWidget->sortByColumn(0, Qt::AscendingOrder);

  ui->add1->setEnabled(false);
  ui->add2->setEnabled(false);

  connect(ui->typographicSingleQuotes, SIGNAL(stateChanged(int)), this, SLOT(enableSingleQuotes(int)));
  connect(ui->typographicDoubleQuotes, SIGNAL(stateChanged(int)), this, SLOT(enableDoubleQuotes(int)));
  connect(ui->singleQuote1, SIGNAL(clicked()), this, SLOT(selectSingleQuoteCharOpen()));
  connect(ui->singleQuote2, SIGNAL(clicked()), this, SLOT(selectSingleQuoteCharClose()));
  connect(ui->singleDefault, SIGNAL(clicked()), this, SLOT(setDefaultSingleQuotes()));
  connect(ui->doubleQuote1, SIGNAL(clicked()), this, SLOT(selectDoubleQuoteCharOpen()));
  connect(ui->doubleQuote2, SIGNAL(clicked()), this, SLOT(selectDoubleQuoteCharClose()));
  connect(ui->doubleDefault, SIGNAL(clicked()), this, SLOT(setDefaultDoubleQuotes()));
  connect(ui->advancedAutocorrection, SIGNAL(stateChanged(int)), this, SLOT(enableAdvAutocorrection(int)));
  connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addAutocorrectEntry()));
  connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeAutocorrectEntry()));
  connect(ui->tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(setFindReplaceText(int, int)));
  connect(ui->find, SIGNAL(textChanged(const QString &)), this, SLOT(enableAddRemoveButton()));
  connect(ui->replace, SIGNAL(textChanged(const QString &)), this, SLOT(enableAddRemoveButton()));
  connect(ui->abbreviation, SIGNAL(textChanged(const QString &)), this, SLOT(abbreviationChanged(const QString &)));
  connect(ui->twoUpperLetter, SIGNAL(textChanged(const QString &)), this, SLOT(twoUpperLetterChanged(const QString &)));
  connect(ui->add1, SIGNAL(clicked()), this, SLOT(addAbbreviationEntry()));
  connect(ui->remove1, SIGNAL(clicked()), this, SLOT(removeAbbreviationEntry()));
  connect(ui->add2, SIGNAL(clicked()), this, SLOT(addTwoUpperLetterEntry()));
  connect(ui->remove2, SIGNAL(clicked()), this, SLOT(removeTwoUpperLetterEntry()));

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
    ui->tableWidget->setRowCount(m_autocorrectEntries.size());
    ui->tableWidget->verticalHeader()->hide();
    QHash<QString, QString>::const_iterator i = m_autocorrectEntries.constBegin();
    int j = 0;
    while (i != m_autocorrectEntries.constEnd()) {
        ui->tableWidget->setItem(j, 0, new QTableWidgetItem(i.key()));
        ui->tableWidget->setItem(j++, 1, new QTableWidgetItem(i.value()));
        ++i;
    }
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->sortByColumn(0, Qt::AscendingOrder);

    enableAdvAutocorrection(ui->advancedAutocorrection->isChecked());
    /* tab 4 - Exceptions */
    m_upperCaseExceptions = mAutoCorrection->upperCaseExceptions();
    m_twoUpperLetterExceptions = mAutoCorrection->twoUpperLetterExceptions();
    ui->abbreviationList->addItems(m_upperCaseExceptions.toList());
}

void KMComposerAutoCorrectionWidget::writeConfig()
{
  if(!mAutoCorrection)
    return;


  mAutoCorrection->setUppercaseFirstCharOfSentence(ui->upperCase->isChecked());
  mAutoCorrection->setFixTwoUppercaseChars(ui->upperUpper->isChecked());
  mAutoCorrection->setSingleSpaces(ui->ignoreDoubleSpace->isChecked());
  mAutoCorrection->setCapitalizeWeekDays(ui->capitalizeDaysName->isChecked());
  mAutoCorrection->setAdvancedAutocorrect(ui->advancedAutocorrection->isChecked());

  mAutoCorrection->setAutocorrectEntries(m_autocorrectEntries);
  mAutoCorrection->setUpperCaseExceptions(m_upperCaseExceptions);
  mAutoCorrection->setTwoUpperLetterExceptions(m_twoUpperLetterExceptions);

  mAutoCorrection->setReplaceDoubleQuotes(ui->typographicDoubleQuotes->isChecked());
  mAutoCorrection->setReplaceSingleQuotes(ui->typographicSingleQuotes->isChecked());
  mAutoCorrection->setTypographicSingleQuotes(m_singleQuotes);
  mAutoCorrection->setTypographicDoubleQuotes(m_doubleQuotes);

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

void KMComposerAutoCorrectionWidget::enableSingleQuotes(int state)
{
    bool enable = state == Qt::Checked;
    ui->singleQuote1->setEnabled(enable);
    ui->singleQuote2->setEnabled(enable);
    ui->singleDefault->setEnabled(enable);
}

void KMComposerAutoCorrectionWidget::enableDoubleQuotes(int state)
{
    bool enable = state == Qt::Checked;
    ui->doubleQuote1->setEnabled(enable);
    ui->doubleQuote2->setEnabled(enable);
    ui->doubleDefault->setEnabled(enable);
}

void KMComposerAutoCorrectionWidget::selectSingleQuoteCharOpen()
{
    CharSelectDialog *dlg = new CharSelectDialog(this);
    dlg->setCurrentChar(m_singleQuotes.begin);
    if (dlg->exec()) {
        m_singleQuotes.begin = dlg->currentChar();
        ui->singleQuote1->setText(m_singleQuotes.begin);
        Q_EMIT changed();
    }
    delete dlg;
}

void KMComposerAutoCorrectionWidget::selectSingleQuoteCharClose()
{
    CharSelectDialog *dlg = new CharSelectDialog(this);
    dlg->setCurrentChar(m_singleQuotes.end);
    if (dlg->exec()) {
        m_singleQuotes.end = dlg->currentChar();
        ui->singleQuote2->setText(m_singleQuotes.end);
        Q_EMIT changed();
    }
    delete dlg;
}

void KMComposerAutoCorrectionWidget::setDefaultSingleQuotes()
{
  m_singleQuotes = mAutoCorrection->typographicDefaultSingleQuotes();
  ui->singleQuote1->setText(m_singleQuotes.begin);
  ui->singleQuote2->setText(m_singleQuotes.end);
}

void KMComposerAutoCorrectionWidget::selectDoubleQuoteCharOpen()
{
    CharSelectDialog *dlg = new CharSelectDialog(this);
    dlg->setCurrentChar(m_doubleQuotes.begin);
    if (dlg->exec()) {
        m_doubleQuotes.begin = dlg->currentChar();
        ui->doubleQuote1->setText(m_doubleQuotes.begin);
        Q_EMIT changed();
    }
    delete dlg;
}

void KMComposerAutoCorrectionWidget::selectDoubleQuoteCharClose()
{
    CharSelectDialog *dlg = new CharSelectDialog(this);
    dlg->setCurrentChar(m_doubleQuotes.end);
    if (dlg->exec()) {
        m_doubleQuotes.end = dlg->currentChar();
        ui->doubleQuote2->setText(m_doubleQuotes.end);
        Q_EMIT changed();
    }
    delete dlg;
}

void KMComposerAutoCorrectionWidget::setDefaultDoubleQuotes()
{
    m_doubleQuotes = mAutoCorrection->typographicDefaultDoubleQuotes();
    ui->doubleQuote1->setText(m_doubleQuotes.begin);
    ui->doubleQuote2->setText(m_doubleQuotes.end);
}

void KMComposerAutoCorrectionWidget::enableAdvAutocorrection(int state)
{
    bool enable = state == Qt::Checked;
    ui->findLabel->setEnabled(enable);
    ui->find->setEnabled(enable);
    ui->specialChar1->setEnabled(enable);
    ui->replaceLabel->setEnabled(enable);
    ui->replace->setEnabled(enable);
    ui->specialChar2->setEnabled(enable);
    ui->addButton->setEnabled(enable);
    ui->removeButton->setEnabled(enable);
    ui->tableWidget->setEnabled(enable);
}


void KMComposerAutoCorrectionWidget::addAutocorrectEntry()
{
    int currentRow = ui->tableWidget->currentRow();
    QString find = ui->find->text();
    bool modify = false;

    // Modify actually, not add, so we want to remove item from hash
    if (currentRow != -1 && find == ui->tableWidget->item(currentRow, 0)->text()) {
        m_autocorrectEntries.remove(find);
        modify = true;
    }

    m_autocorrectEntries.insert(find, ui->replace->text());
    ui->tableWidget->setSortingEnabled(false);
    int size = ui->tableWidget->rowCount();

    if (modify) {
        ui->tableWidget->removeRow(currentRow);
        size--;
    }
    else
        ui->tableWidget->setRowCount(++size);

    QTableWidgetItem *item = new QTableWidgetItem(find);
    ui->tableWidget->setItem(size - 1, 0, item);
    ui->tableWidget->setItem(size - 1, 1, new QTableWidgetItem(ui->replace->text()));

    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->setCurrentCell(item->row(), 0);
}

void KMComposerAutoCorrectionWidget::removeAutocorrectEntry()
{
    ui->tableWidget->setSortingEnabled(false);
    m_autocorrectEntries.remove(ui->find->text());
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
    ui->tableWidget->setSortingEnabled(true);
    Q_EMIT changed();
}

void KMComposerAutoCorrectionWidget::enableAddRemoveButton()
{
    QString find = ui->find->text();
    QString replace = ui->replace->text();
    int currentRow = -1;
    if (m_autocorrectEntries.contains(find)) {
        currentRow = ui->tableWidget->findItems(find, Qt::MatchCaseSensitive).first()->row();
        ui->tableWidget->setCurrentCell(currentRow, 0);
    }
    else
        currentRow = ui->tableWidget->currentRow();

    bool enable = false;
    if (currentRow == -1 || find.isEmpty() || replace.isEmpty()) // disable if no text in find/replace
        enable = !(find.isEmpty() || replace.isEmpty());
    else if (find == ui->tableWidget->item(currentRow, 0)->text()) {
        // We disable add / remove button if no text for the replacement
        enable = !ui->tableWidget->item(currentRow, 1)->text().isEmpty();
        ui->addButton->setText(i18n("&Modify"));
    }
    else if (!ui->tableWidget->item(currentRow, 1)->text().isEmpty()) {
        enable = true;
        ui->addButton->setText(i18n("&Add"));
    }

    if (currentRow != -1) {
    if (replace == ui->tableWidget->item(currentRow, 1)->text())
        ui->addButton->setEnabled(false);
    else
        ui->addButton->setEnabled(enable);
    }
    ui->removeButton->setEnabled(enable);
}

void KMComposerAutoCorrectionWidget::setFindReplaceText(int row, int column)
{
    Q_UNUSED(column);
    ui->find->setText(ui->tableWidget->item(row, 0)->text());
    ui->replace->setText(ui->tableWidget->item(row, 1)->text());
}


void KMComposerAutoCorrectionWidget::abbreviationChanged(const QString &text)
{
    ui->add1->setEnabled(!text.isEmpty());
}

void KMComposerAutoCorrectionWidget::twoUpperLetterChanged(const QString &text)
{
    ui->add2->setEnabled(!text.isEmpty());
}

void KMComposerAutoCorrectionWidget::addAbbreviationEntry()
{
    QString text = ui->abbreviation->text();
    if (!m_upperCaseExceptions.contains(text)) {
        m_upperCaseExceptions.insert(text);
        ui->abbreviationList->addItem(text);
    }
    ui->abbreviation->clear();
}

void KMComposerAutoCorrectionWidget::removeAbbreviationEntry()
{
    int currentRow = ui->abbreviationList->currentRow();
    QListWidgetItem *item = ui->abbreviationList->takeItem(currentRow);
    Q_ASSERT(item);
    m_upperCaseExceptions.remove(item->text());
    delete item;
}

void KMComposerAutoCorrectionWidget::addTwoUpperLetterEntry()
{
    QString text = ui->twoUpperLetter->text();
    if (!m_twoUpperLetterExceptions.contains(text)) {
        m_twoUpperLetterExceptions.insert(text);
        ui->twoUpperLetterList->addItem(text);
    }
    ui->twoUpperLetter->clear();
}

void KMComposerAutoCorrectionWidget::removeTwoUpperLetterEntry()
{
    int currentRow = ui->twoUpperLetterList->currentRow();
    QListWidgetItem *item = ui->twoUpperLetterList->takeItem(currentRow);
    Q_ASSERT(item);
    m_twoUpperLetterExceptions.remove(item->text());
    delete item;
}

CharSelectDialog::CharSelectDialog(QWidget *parent)
  : KDialog(parent)
{
  m_charSelect = new KCharSelect(this, 0,
          KCharSelect::FontCombo | KCharSelect::BlockCombos | KCharSelect::CharacterTable);
  setMainWidget(m_charSelect);
  setCaption(i18n("Select Character"));
}

QChar CharSelectDialog::currentChar() const
{
  return m_charSelect->currentChar();
}

void CharSelectDialog::setCurrentChar(const QChar &c)
{
  m_charSelect->setCurrentChar(c);
}

#include "kmcomposerautocorrectionwidget.moc"
