/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "autocorrectionwidget.h"
#include "widgets/configureimmutablewidgetutils.h"
using namespace PimCommon::ConfigureImmutableWidgetUtils;
#include "autocorrection.h"
#include "ui_autocorrectionwidget.h"
#include "import/importlibreofficeautocorrection.h"
#include "import/importkmailautocorrection.h"
#include "import/importabstractautocorrection.h"

#include "settings/pimcommonsettings.h"
#include <kpimtextedit/selectspecialchardialog.h>

#include <KMessageBox>
#include <KLocalizedString>
#include <QUrl>

#include <QTreeWidgetItem>
#include <QMenu>
#include <QFileDialog>

using namespace PimCommon;

Q_DECLARE_METATYPE(AutoCorrectionWidget::ImportFileType)
class PimCommon::AutoCorrectionWidgetPrivate
{
public:
    AutoCorrectionWidgetPrivate()
        : ui(new Ui::AutoCorrectionWidget),
          mAutoCorrection(Q_NULLPTR),
          mWasChanged(false)
    {

    }
    ~AutoCorrectionWidgetPrivate()
    {
        delete ui;
    }

    AutoCorrection::TypographicQuotes m_singleQuotes;
    AutoCorrection::TypographicQuotes m_doubleQuotes;
    QSet<QString> m_upperCaseExceptions;
    QSet<QString> m_twoUpperLetterExceptions;
    QHash<QString, QString> m_autocorrectEntries;
    Ui::AutoCorrectionWidget *ui;
    AutoCorrection *mAutoCorrection;
    bool mWasChanged;
};

AutoCorrectionWidget::AutoCorrectionWidget(QWidget *parent) :
    QWidget(parent),
    d(new PimCommon::AutoCorrectionWidgetPrivate)
{
    d->ui->setupUi(this);

    d->ui->treeWidget->setSortingEnabled(true);
    d->ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);

    d->ui->add1->setEnabled(false);
    d->ui->add2->setEnabled(false);

    connect(d->ui->autoChangeFormat, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->autoFormatUrl, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->upperCase, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->upperUpper, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->ignoreDoubleSpace, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->autoReplaceNumber, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->capitalizeDaysName, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->advancedAutocorrection, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->enabledAutocorrection, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->typographicSingleQuotes, &QCheckBox::clicked, this, &AutoCorrectionWidget::enableSingleQuotes);
    connect(d->ui->typographicDoubleQuotes, &QCheckBox::clicked, this, &AutoCorrectionWidget::enableDoubleQuotes);
    connect(d->ui->autoSuperScript, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->singleQuote1, &QPushButton::clicked, this, &AutoCorrectionWidget::selectSingleQuoteCharOpen);
    connect(d->ui->singleQuote2, &QPushButton::clicked, this, &AutoCorrectionWidget::selectSingleQuoteCharClose);
    connect(d->ui->singleDefault, &QPushButton::clicked, this, &AutoCorrectionWidget::setDefaultSingleQuotes);
    connect(d->ui->doubleQuote1, &QPushButton::clicked, this, &AutoCorrectionWidget::selectDoubleQuoteCharOpen);
    connect(d->ui->doubleQuote2, &QPushButton::clicked, this, &AutoCorrectionWidget::selectDoubleQuoteCharClose);
    connect(d->ui->doubleDefault, &QPushButton::clicked, this, &AutoCorrectionWidget::setDefaultDoubleQuotes);
    connect(d->ui->advancedAutocorrection, &QCheckBox::clicked, this, &AutoCorrectionWidget::enableAdvAutocorrection);
    connect(d->ui->addButton, &QPushButton::clicked, this, &AutoCorrectionWidget::addAutocorrectEntry);
    connect(d->ui->removeButton, &QPushButton::clicked, this, &AutoCorrectionWidget::removeAutocorrectEntry);
    connect(d->ui->treeWidget, &PimCommon::AutoCorrectionTreeWidget::itemClicked, this, &AutoCorrectionWidget::setFindReplaceText);
    connect(d->ui->treeWidget, &PimCommon::AutoCorrectionTreeWidget::deleteSelectedItems, this, &AutoCorrectionWidget::removeAutocorrectEntry);
    connect(d->ui->treeWidget, &PimCommon::AutoCorrectionTreeWidget::itemSelectionChanged, this, &AutoCorrectionWidget::updateAddRemoveButton);
    connect(d->ui->find, &KLineEdit::textChanged, this, &AutoCorrectionWidget::enableAddRemoveButton);
    connect(d->ui->replace, &KLineEdit::textChanged, this, &AutoCorrectionWidget::enableAddRemoveButton);
    connect(d->ui->abbreviation, &KLineEdit::textChanged, this, &AutoCorrectionWidget::abbreviationChanged);
    connect(d->ui->twoUpperLetter, &KLineEdit::textChanged, this, &AutoCorrectionWidget::twoUpperLetterChanged);
    connect(d->ui->add1, &QPushButton::clicked, this, &AutoCorrectionWidget::addAbbreviationEntry);
    connect(d->ui->remove1, &QPushButton::clicked, this, &AutoCorrectionWidget::removeAbbreviationEntry);
    connect(d->ui->add2, &QPushButton::clicked, this, &AutoCorrectionWidget::addTwoUpperLetterEntry);
    connect(d->ui->remove2, &QPushButton::clicked, this, &AutoCorrectionWidget::removeTwoUpperLetterEntry);
    connect(d->ui->typographicDoubleQuotes, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->typographicSingleQuotes, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->abbreviationList, &PimCommon::AutoCorrectionListWidget::itemSelectionChanged, this, &AutoCorrectionWidget::slotEnableDisableAbreviationList);
    connect(d->ui->abbreviationList, &PimCommon::AutoCorrectionListWidget::deleteSelectedItems, this, &AutoCorrectionWidget::removeAbbreviationEntry);
    connect(d->ui->twoUpperLetterList, &PimCommon::AutoCorrectionListWidget::itemSelectionChanged, this, &AutoCorrectionWidget::slotEnableDisableTwoUpperEntry);
    connect(d->ui->twoUpperLetterList, &PimCommon::AutoCorrectionListWidget::deleteSelectedItems, this, &AutoCorrectionWidget::removeTwoUpperLetterEntry);
    connect(d->ui->autocorrectionLanguage, SIGNAL(activated(int)), SLOT(changeLanguage(int)));
    connect(d->ui->addNonBreakingSpaceInFrench, &QCheckBox::clicked, this, &AutoCorrectionWidget::changed);
    connect(d->ui->twoUpperLetter, &KLineEdit::returnPressed, this, &AutoCorrectionWidget::addTwoUpperLetterEntry);
    connect(d->ui->abbreviation, &KLineEdit::returnPressed, this, &AutoCorrectionWidget::addAbbreviationEntry);
    slotEnableDisableAbreviationList();
    slotEnableDisableTwoUpperEntry();

    QMenu *menu = new QMenu();
    d->ui->importAutoCorrection->setMenu(menu);

    QAction *act = new QAction(i18n("LibreOffice Autocorrection"), this);
    act->setData(QVariant::fromValue(AutoCorrectionWidget::LibreOffice));
    menu->addAction(act);

    act = new QAction(i18n("KMail/Calligra Autocorrection"), this);
    act->setData(QVariant::fromValue(AutoCorrectionWidget::KMail));
    menu->addAction(act);

    connect(menu, &QMenu::triggered, this, &AutoCorrectionWidget::slotImportAutoCorrection);

    connect(d->ui->exportAutoCorrection, &QPushButton::clicked, this, &AutoCorrectionWidget::slotExportAutoCorrection);
}

AutoCorrectionWidget::~AutoCorrectionWidget()
{
    delete d;
}

void AutoCorrectionWidget::setAutoCorrection(AutoCorrection *autoCorrect)
{
    d->mAutoCorrection = autoCorrect;
    setLanguage(d->ui->autocorrectionLanguage->language());
}

void AutoCorrectionWidget::loadConfig()
{
    if (!d->mAutoCorrection) {
        return;
    }

    d->ui->autoChangeFormat->setChecked(d->mAutoCorrection->isAutoBoldUnderline());
    d->ui->autoFormatUrl->setChecked(d->mAutoCorrection->isAutoFormatUrl());
    d->ui->enabledAutocorrection->setChecked(d->mAutoCorrection->isEnabledAutoCorrection());
    d->ui->upperCase->setChecked(d->mAutoCorrection->isUppercaseFirstCharOfSentence());
    d->ui->upperUpper->setChecked(d->mAutoCorrection->isFixTwoUppercaseChars());
    d->ui->ignoreDoubleSpace->setChecked(d->mAutoCorrection->isSingleSpaces());
    d->ui->autoReplaceNumber->setChecked(d->mAutoCorrection->isAutoFractions());
    d->ui->capitalizeDaysName->setChecked(d->mAutoCorrection->isCapitalizeWeekDays());
    d->ui->advancedAutocorrection->setChecked(d->mAutoCorrection->isAdvancedAutocorrect());
    d->ui->autoSuperScript->setChecked(d->mAutoCorrection->isSuperScript());
    d->ui->typographicDoubleQuotes->setChecked(d->mAutoCorrection->isReplaceDoubleQuotes());
    d->ui->typographicSingleQuotes->setChecked(d->mAutoCorrection->isReplaceSingleQuotes());
    d->ui->addNonBreakingSpaceInFrench->setChecked(d->mAutoCorrection->isAddNonBreakingSpace());
    loadAutoCorrectionAndException();
    d->mWasChanged = false;
}

void AutoCorrectionWidget::loadAutoCorrectionAndException()
{
    /* tab 2 - Custom Quotes */
    d->m_singleQuotes = d->mAutoCorrection->typographicSingleQuotes();
    d->ui->singleQuote1->setText(d->m_singleQuotes.begin);
    d->ui->singleQuote2->setText(d->m_singleQuotes.end);
    d->m_doubleQuotes = d->mAutoCorrection->typographicDoubleQuotes();
    d->ui->doubleQuote1->setText(d->m_doubleQuotes.begin);
    d->ui->doubleQuote2->setText(d->m_doubleQuotes.end);
    enableSingleQuotes(d->ui->typographicSingleQuotes->isChecked());
    enableDoubleQuotes(d->ui->typographicDoubleQuotes->isChecked());

    /* tab 3 - Advanced Autocorrection */
    d->m_autocorrectEntries = d->mAutoCorrection->autocorrectEntries();
    addAutoCorrectEntries();

    enableAdvAutocorrection(d->ui->advancedAutocorrection->isChecked());
    /* tab 4 - Exceptions */
    d->m_upperCaseExceptions = d->mAutoCorrection->upperCaseExceptions();
    d->m_twoUpperLetterExceptions = d->mAutoCorrection->twoUpperLetterExceptions();

    d->ui->twoUpperLetterList->clear();
    d->ui->twoUpperLetterList->addItems(d->m_twoUpperLetterExceptions.toList());

    d->ui->abbreviationList->clear();
    d->ui->abbreviationList->addItems(d->m_upperCaseExceptions.toList());

}

void AutoCorrectionWidget::addAutoCorrectEntries()
{
    d->ui->treeWidget->clear();
    QHash<QString, QString>::const_iterator i = d->m_autocorrectEntries.constBegin();
    QTreeWidgetItem *item = Q_NULLPTR;
    while (i != d->m_autocorrectEntries.constEnd()) {
        item = new QTreeWidgetItem(d->ui->treeWidget, item);
        item->setText(0, i.key());
        item->setText(1, i.value());
        ++i;
    }
    d->ui->treeWidget->setSortingEnabled(true);
    d->ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
}

void AutoCorrectionWidget::writeConfig()
{
    if (!d->mAutoCorrection) {
        return;
    }
    d->mAutoCorrection->setAutoBoldUnderline(d->ui->autoChangeFormat->isChecked());
    d->mAutoCorrection->setAutoFormatUrl(d->ui->autoFormatUrl->isChecked());
    d->mAutoCorrection->setEnabledAutoCorrection(d->ui->enabledAutocorrection->isChecked());
    d->mAutoCorrection->setUppercaseFirstCharOfSentence(d->ui->upperCase->isChecked());
    d->mAutoCorrection->setFixTwoUppercaseChars(d->ui->upperUpper->isChecked());
    d->mAutoCorrection->setSingleSpaces(d->ui->ignoreDoubleSpace->isChecked());
    d->mAutoCorrection->setCapitalizeWeekDays(d->ui->capitalizeDaysName->isChecked());
    d->mAutoCorrection->setAdvancedAutocorrect(d->ui->advancedAutocorrection->isChecked());
    d->mAutoCorrection->setSuperScript(d->ui->autoSuperScript->isChecked());

    d->mAutoCorrection->setAutoFractions(d->ui->autoReplaceNumber->isChecked());

    d->mAutoCorrection->setAutocorrectEntries(d->m_autocorrectEntries);
    d->mAutoCorrection->setUpperCaseExceptions(d->m_upperCaseExceptions);
    d->mAutoCorrection->setTwoUpperLetterExceptions(d->m_twoUpperLetterExceptions);

    d->mAutoCorrection->setReplaceDoubleQuotes(d->ui->typographicDoubleQuotes->isChecked());
    d->mAutoCorrection->setReplaceSingleQuotes(d->ui->typographicSingleQuotes->isChecked());
    d->mAutoCorrection->setTypographicSingleQuotes(d->m_singleQuotes);
    d->mAutoCorrection->setTypographicDoubleQuotes(d->m_doubleQuotes);
    d->mAutoCorrection->setAddNonBreakingSpace(d->ui->addNonBreakingSpaceInFrench->isChecked());
    d->mAutoCorrection->writeConfig();
    d->mWasChanged = false;
}

void AutoCorrectionWidget::resetToDefault()
{
    d->ui->autoChangeFormat->setChecked(false);
    d->ui->autoFormatUrl->setChecked(false);
    d->ui->upperCase->setChecked(false);
    d->ui->upperUpper->setChecked(false);
    d->ui->ignoreDoubleSpace->setChecked(false);
    d->ui->capitalizeDaysName->setChecked(false);
    d->ui->advancedAutocorrection->setChecked(false);
    d->ui->typographicDoubleQuotes->setChecked(false);
    d->ui->typographicSingleQuotes->setChecked(false);
    d->ui->autoSuperScript->setChecked(false);
    d->ui->autoReplaceNumber->setChecked(false);
    d->ui->typographicDoubleQuotes->setChecked(false);
    d->ui->typographicSingleQuotes->setChecked(false);
    d->ui->addNonBreakingSpaceInFrench->setChecked(false);

    loadGlobalAutoCorrectionAndException();
}

void AutoCorrectionWidget::enableSingleQuotes(bool state)
{
    d->ui->singleQuote1->setEnabled(state);
    d->ui->singleQuote2->setEnabled(state);
    d->ui->singleDefault->setEnabled(state);
}

void AutoCorrectionWidget::enableDoubleQuotes(bool state)
{
    d->ui->doubleQuote1->setEnabled(state);
    d->ui->doubleQuote2->setEnabled(state);
    d->ui->doubleDefault->setEnabled(state);
}

void AutoCorrectionWidget::selectSingleQuoteCharOpen()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(this);
    dlg.setCurrentChar(d->m_singleQuotes.begin);
    dlg.showSelectButton(false);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        d->m_singleQuotes.begin = dlg.currentChar();
        d->ui->singleQuote1->setText(d->m_singleQuotes.begin);
        emitChanged();
    }
}

void AutoCorrectionWidget::selectSingleQuoteCharClose()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(this);
    dlg.showSelectButton(false);
    dlg.setCurrentChar(d->m_singleQuotes.end);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        d->m_singleQuotes.end = dlg.currentChar();
        d->ui->singleQuote2->setText(d->m_singleQuotes.end);
        emitChanged();
    }
}

void AutoCorrectionWidget::setDefaultSingleQuotes()
{
    d->m_singleQuotes = d->mAutoCorrection->typographicDefaultSingleQuotes();
    d->ui->singleQuote1->setText(d->m_singleQuotes.begin);
    d->ui->singleQuote2->setText(d->m_singleQuotes.end);
    emitChanged();
}

void AutoCorrectionWidget::selectDoubleQuoteCharOpen()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(this);
    dlg.showSelectButton(false);
    dlg.setCurrentChar(d->m_doubleQuotes.begin);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        d->m_doubleQuotes.begin = dlg.currentChar();
        d->ui->doubleQuote1->setText(d->m_doubleQuotes.begin);
        emitChanged();
    }
}

void AutoCorrectionWidget::selectDoubleQuoteCharClose()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(this);
    dlg.showSelectButton(false);
    dlg.setCurrentChar(d->m_doubleQuotes.end);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        d->m_doubleQuotes.end = dlg.currentChar();
        d->ui->doubleQuote2->setText(d->m_doubleQuotes.end);
        emitChanged();
    }
}

void AutoCorrectionWidget::setDefaultDoubleQuotes()
{
    d->m_doubleQuotes = d->mAutoCorrection->typographicDefaultDoubleQuotes();
    d->ui->doubleQuote1->setText(d->m_doubleQuotes.begin);
    d->ui->doubleQuote2->setText(d->m_doubleQuotes.end);
    emitChanged();
}

void AutoCorrectionWidget::enableAdvAutocorrection(bool state)
{
    d->ui->findLabel->setEnabled(state);
    d->ui->find->setEnabled(state);
    d->ui->replaceLabel->setEnabled(state);
    d->ui->replace->setEnabled(state);

    const QString find = d->ui->find->text();
    const QString replace = d->ui->replace->text();

    d->ui->addButton->setEnabled(state && !find.isEmpty() && !replace.isEmpty());
    d->ui->removeButton->setEnabled(state && d->ui->treeWidget->currentItem());
    d->ui->treeWidget->setEnabled(state);
}

void AutoCorrectionWidget::addAutocorrectEntry()
{
    QTreeWidgetItem *item = d->ui->treeWidget->currentItem();
    const QString find = d->ui->find->text();
    const QString replace = d->ui->replace->text();
    if (find == replace) {
        KMessageBox::error(this, i18n("\"Replace\" string is the same as \"Find\" string."), i18n("Add Autocorrection Entry"));
        return;
    }

    bool modify = false;

    // Modify actually, not add, so we want to remove item from hash
    if (item && (find == item->text(0))) {
        d->m_autocorrectEntries.remove(find);
        modify = true;
    }

    d->m_autocorrectEntries.insert(find, replace);
    d->ui->treeWidget->setSortingEnabled(false);
    if (modify) {
        item->setText(0, find);
        item->setText(1, replace);
    } else {
        item = new QTreeWidgetItem(d->ui->treeWidget, item);
        item->setText(0, find);
        item->setText(1, replace);
    }

    d->ui->treeWidget->setSortingEnabled(true);
    d->ui->treeWidget->setCurrentItem(item);
    emitChanged();
}

void AutoCorrectionWidget::removeAutocorrectEntry()
{
    QList<QTreeWidgetItem *> listItems = d->ui->treeWidget->selectedItems();
    if (listItems.isEmpty()) {
        return;
    }
    Q_FOREACH (QTreeWidgetItem *item, listItems) {
        QTreeWidgetItem *below = d->ui->treeWidget->itemBelow(item);

        QString findStr;
        if (below) {
            //qCDebug(PIMCOMMON_LOG) << "below";
            findStr = item->text(0);
            delete item;
            item = Q_NULLPTR;
        } else if (d->ui->treeWidget->topLevelItemCount() > 0) {
            findStr = item->text(0);
            delete item;
            item = Q_NULLPTR;
        }
        if (!findStr.isEmpty()) {
            d->m_autocorrectEntries.remove(findStr);
        }
    }
    d->ui->treeWidget->setSortingEnabled(false);

    emitChanged();
}

void AutoCorrectionWidget::updateAddRemoveButton()
{
    QList<QTreeWidgetItem *> listItems = d->ui->treeWidget->selectedItems();
    d->ui->removeButton->setEnabled(!listItems.isEmpty());
}

void AutoCorrectionWidget::enableAddRemoveButton()
{
    const QString find = d->ui->find->text();
    const QString replace = d->ui->replace->text();

    QTreeWidgetItem *item = Q_NULLPTR;
    if (d->m_autocorrectEntries.contains(find)) {
        item = d->ui->treeWidget->findItems(find, Qt::MatchCaseSensitive).at(0);
    }

    bool enable = false;
    if (find.isEmpty() || replace.isEmpty()) { // disable if no text in find/replace
        enable = !(find.isEmpty() || replace.isEmpty());
    } else if (item && find == item->text(0)) {
        // We disable add / remove button if no text for the replacement
        enable = !item->text(1).isEmpty();
        d->ui->addButton->setText(i18n("&Modify"));
    } else if (!item || !item->text(1).isEmpty()) {
        enable = true;
        d->ui->addButton->setText(i18n("&Add"));
    }

    if (item && replace == item->text(1)) {
        d->ui->addButton->setEnabled(false);
    } else {
        d->ui->addButton->setEnabled(enable);
    }
    d->ui->removeButton->setEnabled(enable);

}

void AutoCorrectionWidget::setFindReplaceText(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    d->ui->find->setText(item->text(0));
    d->ui->replace->setText(item->text(1));
}

void AutoCorrectionWidget::abbreviationChanged(const QString &text)
{
    d->ui->add1->setEnabled(!text.isEmpty());
}

void AutoCorrectionWidget::twoUpperLetterChanged(const QString &text)
{
    d->ui->add2->setEnabled(!text.isEmpty());
}

void AutoCorrectionWidget::addAbbreviationEntry()
{
    const QString text = d->ui->abbreviation->text();
    if (text.isEmpty()) {
        return;
    }
    if (!d->m_upperCaseExceptions.contains(text)) {
        d->m_upperCaseExceptions.insert(text);
        d->ui->abbreviationList->addItem(text);
    }
    d->ui->abbreviation->clear();
    slotEnableDisableAbreviationList();
    emitChanged();
}

void AutoCorrectionWidget::removeAbbreviationEntry()
{
    QList<QListWidgetItem *> listItem = d->ui->abbreviationList->selectedItems();
    if (listItem.isEmpty()) {
        return;
    }
    Q_FOREACH (QListWidgetItem *item, listItem) {
        d->m_upperCaseExceptions.remove(item->text());
        delete item;
    }
    slotEnableDisableAbreviationList();
    emitChanged();
}

void AutoCorrectionWidget::addTwoUpperLetterEntry()
{
    const QString text = d->ui->twoUpperLetter->text();
    if (text.isEmpty()) {
        return;
    }
    if (!d->m_twoUpperLetterExceptions.contains(text)) {
        d->m_twoUpperLetterExceptions.insert(text);
        d->ui->twoUpperLetterList->addItem(text);
        emitChanged();
    }
    slotEnableDisableTwoUpperEntry();
    d->ui->twoUpperLetter->clear();

}

void AutoCorrectionWidget::removeTwoUpperLetterEntry()
{
    QList<QListWidgetItem *> listItem = d->ui->twoUpperLetterList->selectedItems();
    if (listItem.isEmpty()) {
        return;
    }
    Q_FOREACH (QListWidgetItem *item, listItem) {
        d->m_twoUpperLetterExceptions.remove(item->text());
        delete item;
    }
    slotEnableDisableTwoUpperEntry();
    emitChanged();
}

void AutoCorrectionWidget::slotEnableDisableAbreviationList()
{
    const bool enable = (!d->ui->abbreviationList->selectedItems().isEmpty());
    d->ui->add1->setEnabled(!d->ui->abbreviation->text().isEmpty());
    d->ui->remove1->setEnabled(enable);
}

void AutoCorrectionWidget::slotEnableDisableTwoUpperEntry()
{
    const bool enable = (!d->ui->twoUpperLetterList->selectedItems().isEmpty());
    d->ui->add2->setEnabled(!d->ui->twoUpperLetter->text().isEmpty());
    d->ui->remove2->setEnabled(enable);
}

void AutoCorrectionWidget::slotImportAutoCorrection(QAction *act)
{
    if (act) {
        AutoCorrectionWidget::ImportFileType type = act->data().value<AutoCorrectionWidget::ImportFileType>();
        QString title;
        QString filter;
        switch (type) {
        case AutoCorrectionWidget::LibreOffice:
            title = i18n("Import LibreOffice Autocorrection");
            filter = i18n("LibreOffice Autocorrection File (*.data)");
            break;
        case AutoCorrectionWidget::KMail:
            title = i18n("Import KMail Autocorrection");
            filter = i18n("KMail Autocorrection File (*.xml)");
            break;
        }
        const QString fileName = QFileDialog::getOpenFileName(this, title,  QString(), filter);
        if (!fileName.isEmpty()) {
            PimCommon::ImportAbstractAutocorrection *importAutoCorrection = Q_NULLPTR;
            switch (type) {
            case AutoCorrectionWidget::LibreOffice:
                importAutoCorrection = new PimCommon::ImportLibreOfficeAutocorrection(this);
                break;
            case AutoCorrectionWidget::KMail:
                importAutoCorrection = new PimCommon::ImportKMailAutocorrection(this);
                break;
            default:
                return;
            }
            if (importAutoCorrection->import(fileName, ImportAbstractAutocorrection::All)) {
                d->m_autocorrectEntries = importAutoCorrection->autocorrectEntries();
                addAutoCorrectEntries();

                enableAdvAutocorrection(d->ui->advancedAutocorrection->isChecked());

                d->m_upperCaseExceptions = importAutoCorrection->upperCaseExceptions();
                d->m_twoUpperLetterExceptions = importAutoCorrection->twoUpperLetterExceptions();

                d->ui->twoUpperLetterList->clear();
                d->ui->twoUpperLetterList->addItems(d->m_twoUpperLetterExceptions.toList());

                d->ui->abbreviationList->clear();
                d->ui->abbreviationList->addItems(d->m_upperCaseExceptions.toList());
            }
            delete importAutoCorrection;
        }
    }
}

void AutoCorrectionWidget::setLanguage(const QString &lang)
{
    d->mAutoCorrection->setLanguage(lang);
    loadAutoCorrectionAndException();
    d->mWasChanged = false;
}

void AutoCorrectionWidget::changeLanguage(int index)
{
    if (index == -1) {
        return;
    }
    if (d->mWasChanged) {
        const int rc = KMessageBox::warningYesNo(this, i18n("Language was changed, do you want to save config for previous language?"), i18n("Save config"));
        if (rc == KMessageBox::Yes) {
            writeConfig();
        }
    }
    const QString lang = d->ui->autocorrectionLanguage->itemData(index).toString();
    d->mAutoCorrection->setLanguage(lang);
    loadAutoCorrectionAndException();
    d->mWasChanged = false;
}

void AutoCorrectionWidget::emitChanged()
{
    d->mWasChanged = true;
    Q_EMIT changed();
}

void AutoCorrectionWidget::loadGlobalAutoCorrectionAndException()
{
    const QString lang = d->ui->autocorrectionLanguage->itemData(d->ui->autocorrectionLanguage->currentIndex()).toString();
    d->mAutoCorrection->setLanguage(lang, true);
    loadAutoCorrectionAndException();
    d->mWasChanged = true;
    Q_EMIT changed();
}

void AutoCorrectionWidget::slotExportAutoCorrection()
{
    const QString saveUrl = QFileDialog::getSaveFileName(this, i18n("Export Autocorrection File"), QDir::homePath());
    if (saveUrl.isEmpty()) {
        return;
    }
    d->mAutoCorrection->writeAutoCorrectionXmlFile(saveUrl);
}

