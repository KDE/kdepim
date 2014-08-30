/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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
#include "pimcommon/widgets/configureimmutablewidgetutils.h"
using namespace PimCommon::ConfigureImmutableWidgetUtils;
#include "autocorrection.h"
#include "ui_autocorrectionwidget.h"
#include "import/importlibreofficeautocorrection.h"
#include "import/importkmailautocorrection.h"
#include "import/importabstractautocorrection.h"

#include "settings/pimcommonsettings.h"
#include <kpimtextedit/selectspecialchardialog.h>

#include <KFileDialog>
#include <KMessageBox>
#include <KLocalizedString>
#include <KUrl>

#include <QTreeWidgetItem>
#include <QMenu>
#include <QFileDialog>

using namespace PimCommon;

Q_DECLARE_METATYPE(AutoCorrectionWidget::ImportFileType)

AutoCorrectionWidget::AutoCorrectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutoCorrectionWidget),
    mAutoCorrection(0),
    mWasChanged(false)
{
    ui->setupUi(this);

    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);

    ui->add1->setEnabled(false);
    ui->add2->setEnabled(false);

    connect(ui->autoChangeFormat,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->autoFormatUrl,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->upperCase,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->upperUpper,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->ignoreDoubleSpace,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->autoReplaceNumber,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->capitalizeDaysName,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->advancedAutocorrection,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->enabledAutocorrection,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->typographicSingleQuotes, &QCheckBox::clicked, this, &AutoCorrectionWidget::enableSingleQuotes);
    connect(ui->typographicDoubleQuotes, &QCheckBox::clicked, this, &AutoCorrectionWidget::enableDoubleQuotes);
    connect(ui->autoSuperScript,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->singleQuote1, &QPushButton::clicked, this, &AutoCorrectionWidget::selectSingleQuoteCharOpen);
    connect(ui->singleQuote2, &QPushButton::clicked, this, &AutoCorrectionWidget::selectSingleQuoteCharClose);
    connect(ui->singleDefault, &QPushButton::clicked, this, &AutoCorrectionWidget::setDefaultSingleQuotes);
    connect(ui->doubleQuote1, &QPushButton::clicked, this, &AutoCorrectionWidget::selectDoubleQuoteCharOpen);
    connect(ui->doubleQuote2, &QPushButton::clicked, this, &AutoCorrectionWidget::selectDoubleQuoteCharClose);
    connect(ui->doubleDefault, &QPushButton::clicked, this, &AutoCorrectionWidget::setDefaultDoubleQuotes);
    connect(ui->advancedAutocorrection, &QCheckBox::clicked, this, &AutoCorrectionWidget::enableAdvAutocorrection);
    connect(ui->addButton, &QPushButton::clicked, this, &AutoCorrectionWidget::addAutocorrectEntry);
    connect(ui->removeButton, &QPushButton::clicked, this, &AutoCorrectionWidget::removeAutocorrectEntry);
    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(setFindReplaceText(QTreeWidgetItem*,int)));
    connect(ui->treeWidget, &PimCommon::AutoCorrectionTreeWidget::deleteSelectedItems, this, &AutoCorrectionWidget::removeAutocorrectEntry);
    connect(ui->treeWidget, &PimCommon::AutoCorrectionTreeWidget::itemSelectionChanged, this, &AutoCorrectionWidget::updateAddRemoveButton);
    connect(ui->find, &KLineEdit::textChanged, this, &AutoCorrectionWidget::enableAddRemoveButton);
    connect(ui->replace, &KLineEdit::textChanged, this, &AutoCorrectionWidget::enableAddRemoveButton);
    connect(ui->abbreviation, &KLineEdit::textChanged, this, &AutoCorrectionWidget::abbreviationChanged);
    connect(ui->twoUpperLetter, &KLineEdit::textChanged, this, &AutoCorrectionWidget::twoUpperLetterChanged);
    connect(ui->add1, &QPushButton::clicked, this, &AutoCorrectionWidget::addAbbreviationEntry);
    connect(ui->remove1, &QPushButton::clicked, this, &AutoCorrectionWidget::removeAbbreviationEntry);
    connect(ui->add2, &QPushButton::clicked, this, &AutoCorrectionWidget::addTwoUpperLetterEntry);
    connect(ui->remove2, &QPushButton::clicked, this, &AutoCorrectionWidget::removeTwoUpperLetterEntry);
    connect(ui->typographicDoubleQuotes,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->typographicSingleQuotes,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->abbreviationList, &PimCommon::AutoCorrectionListWidget::itemSelectionChanged, this, &AutoCorrectionWidget::slotEnableDisableAbreviationList);
    connect(ui->abbreviationList, &PimCommon::AutoCorrectionListWidget::deleteSelectedItems, this, &AutoCorrectionWidget::removeAbbreviationEntry);
    connect(ui->twoUpperLetterList, &PimCommon::AutoCorrectionListWidget::itemSelectionChanged, this, &AutoCorrectionWidget::slotEnableDisableTwoUpperEntry);
    connect(ui->twoUpperLetterList, &PimCommon::AutoCorrectionListWidget::deleteSelectedItems, this, &AutoCorrectionWidget::removeTwoUpperLetterEntry);
    connect(ui->autocorrectionLanguage,SIGNAL(activated(int)),SLOT(changeLanguage(int)));
    connect(ui->addNonBreakingSpaceInFrench,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->twoUpperLetter, &KLineEdit::returnPressed, this, &AutoCorrectionWidget::addTwoUpperLetterEntry);
    connect(ui->abbreviation, &KLineEdit::returnPressed, this, &AutoCorrectionWidget::addAbbreviationEntry);
    slotEnableDisableAbreviationList();
    slotEnableDisableTwoUpperEntry();

    QMenu *menu = new QMenu();
    ui->importAutoCorrection->setMenu( menu );

    QAction *act = new QAction( i18n( "LibreOffice Autocorrection" ), this );
    act->setData( QVariant::fromValue( AutoCorrectionWidget::LibreOffice ) );
    menu->addAction( act );

    act = new QAction( i18n( "KMail/Calligra Autocorrection" ), this );
    act->setData( QVariant::fromValue( AutoCorrectionWidget::KMail ) );
    menu->addAction( act );

    connect(menu, &QMenu::triggered, this, &AutoCorrectionWidget::slotImportAutoCorrection);

    connect(ui->exportAutoCorrection, &QPushButton::clicked, this, &AutoCorrectionWidget::slotExportAutoCorrection);
}

AutoCorrectionWidget::~AutoCorrectionWidget()
{
    delete ui;
}

void AutoCorrectionWidget::setAutoCorrection(AutoCorrection * autoCorrect)
{
    mAutoCorrection = autoCorrect;
    setLanguage(ui->autocorrectionLanguage->language());
}

void AutoCorrectionWidget::loadConfig()
{
    if (!mAutoCorrection)
        return;

    ui->autoChangeFormat->setChecked(mAutoCorrection->isAutoBoldUnderline());
    ui->autoFormatUrl->setChecked(mAutoCorrection->isAutoFormatUrl());
    ui->enabledAutocorrection->setChecked(mAutoCorrection->isEnabledAutoCorrection());
    ui->upperCase->setChecked(mAutoCorrection->isUppercaseFirstCharOfSentence());
    ui->upperUpper->setChecked(mAutoCorrection->isFixTwoUppercaseChars());
    ui->ignoreDoubleSpace->setChecked(mAutoCorrection->isSingleSpaces());
    ui->autoReplaceNumber->setChecked(mAutoCorrection->isAutoFractions());
    ui->capitalizeDaysName->setChecked(mAutoCorrection->isCapitalizeWeekDays());
    ui->advancedAutocorrection->setChecked(mAutoCorrection->isAdvancedAutocorrect());
    ui->autoSuperScript->setChecked(mAutoCorrection->isSuperScript());
    ui->typographicDoubleQuotes->setChecked(mAutoCorrection->isReplaceDoubleQuotes());
    ui->typographicSingleQuotes->setChecked(mAutoCorrection->isReplaceSingleQuotes());
    ui->addNonBreakingSpaceInFrench->setChecked(mAutoCorrection->isAddNonBreakingSpace());
    loadAutoCorrectionAndException();
    mWasChanged = false;
}

void AutoCorrectionWidget::loadAutoCorrectionAndException()
{
    /* tab 2 - Custom Quotes */
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
    addAutoCorrectEntries();

    enableAdvAutocorrection(ui->advancedAutocorrection->isChecked());
    /* tab 4 - Exceptions */
    m_upperCaseExceptions = mAutoCorrection->upperCaseExceptions();
    m_twoUpperLetterExceptions = mAutoCorrection->twoUpperLetterExceptions();

    ui->twoUpperLetterList->clear();
    ui->twoUpperLetterList->addItems(m_twoUpperLetterExceptions.toList());

    ui->abbreviationList->clear();
    ui->abbreviationList->addItems(m_upperCaseExceptions.toList());

}

void AutoCorrectionWidget::addAutoCorrectEntries()
{
    ui->treeWidget->clear();
    QHash<QString, QString>::const_iterator i = m_autocorrectEntries.constBegin();
    QTreeWidgetItem * item = 0;
    while (i != m_autocorrectEntries.constEnd()) {
        item = new QTreeWidgetItem( ui->treeWidget, item );
        item->setText( 0, i.key() );
        item->setText( 1, i.value() );
        ++i;
    }
    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
}

void AutoCorrectionWidget::writeConfig()
{
    if(!mAutoCorrection)
        return;
    mAutoCorrection->setAutoBoldUnderline(ui->autoChangeFormat->isChecked());
    mAutoCorrection->setAutoFormatUrl(ui->autoFormatUrl->isChecked());
    mAutoCorrection->setEnabledAutoCorrection(ui->enabledAutocorrection->isChecked());
    mAutoCorrection->setUppercaseFirstCharOfSentence(ui->upperCase->isChecked());
    mAutoCorrection->setFixTwoUppercaseChars(ui->upperUpper->isChecked());
    mAutoCorrection->setSingleSpaces(ui->ignoreDoubleSpace->isChecked());
    mAutoCorrection->setCapitalizeWeekDays(ui->capitalizeDaysName->isChecked());
    mAutoCorrection->setAdvancedAutocorrect(ui->advancedAutocorrection->isChecked());
    mAutoCorrection->setSuperScript(ui->autoSuperScript->isChecked());

    mAutoCorrection->setAutoFractions(ui->autoReplaceNumber->isChecked());

    mAutoCorrection->setAutocorrectEntries(m_autocorrectEntries);
    mAutoCorrection->setUpperCaseExceptions(m_upperCaseExceptions);
    mAutoCorrection->setTwoUpperLetterExceptions(m_twoUpperLetterExceptions);

    mAutoCorrection->setReplaceDoubleQuotes(ui->typographicDoubleQuotes->isChecked());
    mAutoCorrection->setReplaceSingleQuotes(ui->typographicSingleQuotes->isChecked());
    mAutoCorrection->setTypographicSingleQuotes(m_singleQuotes);
    mAutoCorrection->setTypographicDoubleQuotes(m_doubleQuotes);
    mAutoCorrection->setAddNonBreakingSpace(ui->addNonBreakingSpaceInFrench->isChecked());
    mAutoCorrection->writeConfig();
    mWasChanged = false;
}

void AutoCorrectionWidget::resetToDefault()
{
    ui->autoChangeFormat->setChecked(false);
    ui->autoFormatUrl->setChecked(false);
    ui->upperCase->setChecked(false);
    ui->upperUpper->setChecked(false);
    ui->ignoreDoubleSpace->setChecked(false);
    ui->capitalizeDaysName->setChecked(false);
    ui->advancedAutocorrection->setChecked(false);
    ui->typographicDoubleQuotes->setChecked(false);
    ui->typographicSingleQuotes->setChecked(false);
    ui->autoSuperScript->setChecked(false);
    ui->autoReplaceNumber->setChecked(false);
    ui->typographicDoubleQuotes->setChecked(false);
    ui->typographicSingleQuotes->setChecked(false);
    ui->addNonBreakingSpaceInFrench->setChecked(false);

    loadGlobalAutoCorrectionAndException();
}

void AutoCorrectionWidget::enableSingleQuotes(bool state)
{
    ui->singleQuote1->setEnabled(state);
    ui->singleQuote2->setEnabled(state);
    ui->singleDefault->setEnabled(state);
}

void AutoCorrectionWidget::enableDoubleQuotes(bool state)
{
    ui->doubleQuote1->setEnabled(state);
    ui->doubleQuote2->setEnabled(state);
    ui->doubleDefault->setEnabled(state);
}

void AutoCorrectionWidget::selectSingleQuoteCharOpen()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(this);
    dlg.setCurrentChar(m_singleQuotes.begin);
    dlg.showSelectButton(false);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        m_singleQuotes.begin = dlg.currentChar();
        ui->singleQuote1->setText(m_singleQuotes.begin);
        emitChanged();
    }
}

void AutoCorrectionWidget::selectSingleQuoteCharClose()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(this);
    dlg.showSelectButton(false);
    dlg.setCurrentChar(m_singleQuotes.end);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        m_singleQuotes.end = dlg.currentChar();
        ui->singleQuote2->setText(m_singleQuotes.end);
        emitChanged();
    }
}

void AutoCorrectionWidget::setDefaultSingleQuotes()
{
    m_singleQuotes = mAutoCorrection->typographicDefaultSingleQuotes();
    ui->singleQuote1->setText(m_singleQuotes.begin);
    ui->singleQuote2->setText(m_singleQuotes.end);
    emitChanged();
}

void AutoCorrectionWidget::selectDoubleQuoteCharOpen()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(this);
    dlg.showSelectButton(false);
    dlg.setCurrentChar(m_doubleQuotes.begin);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        m_doubleQuotes.begin = dlg.currentChar();
        ui->doubleQuote1->setText(m_doubleQuotes.begin);
        emitChanged();
    }
}

void AutoCorrectionWidget::selectDoubleQuoteCharClose()
{
    KPIMTextEdit::SelectSpecialCharDialog dlg(this);
    dlg.showSelectButton(false);
    dlg.setCurrentChar(m_doubleQuotes.end);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        m_doubleQuotes.end = dlg.currentChar();
        ui->doubleQuote2->setText(m_doubleQuotes.end);
        emitChanged();
    }
}

void AutoCorrectionWidget::setDefaultDoubleQuotes()
{
    m_doubleQuotes = mAutoCorrection->typographicDefaultDoubleQuotes();
    ui->doubleQuote1->setText(m_doubleQuotes.begin);
    ui->doubleQuote2->setText(m_doubleQuotes.end);
    emitChanged();
}

void AutoCorrectionWidget::enableAdvAutocorrection(bool state)
{
    ui->findLabel->setEnabled(state);
    ui->find->setEnabled(state);
    ui->replaceLabel->setEnabled(state);
    ui->replace->setEnabled(state);

    const QString find = ui->find->text();
    const QString replace = ui->replace->text();

    ui->addButton->setEnabled(state && !find.isEmpty() && !replace.isEmpty());
    ui->removeButton->setEnabled(state && ui->treeWidget->currentItem ());
    ui->treeWidget->setEnabled(state);
}


void AutoCorrectionWidget::addAutocorrectEntry()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem ();
    const QString find = ui->find->text();
    const QString replace = ui->replace->text();
    if (find == replace ) {
        KMessageBox::error( this, i18n("\"Replace\" string is the same as \"Find\" string."),i18n( "Add Autocorrection Entry" ) );
        return;
    }

    bool modify = false;

    // Modify actually, not add, so we want to remove item from hash
    if (item && (find == item->text(0))) {
        m_autocorrectEntries.remove(find);
        modify = true;
    }

    m_autocorrectEntries.insert(find, replace);
    ui->treeWidget->setSortingEnabled(false);
    if (modify) {
        item->setText(0,find);
        item->setText(1,replace);
    } else {
        item = new QTreeWidgetItem( ui->treeWidget, item );
        item->setText( 0, find );
        item->setText( 1, replace );
    }

    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->setCurrentItem(item);
    emitChanged();
}

void AutoCorrectionWidget::removeAutocorrectEntry()
{
    QList<QTreeWidgetItem *> listItems = ui->treeWidget->selectedItems ();
    if (listItems.isEmpty())
        return;
    Q_FOREACH (QTreeWidgetItem *item, listItems) {
        QTreeWidgetItem *below = ui->treeWidget->itemBelow( item );

        QString findStr;
        if ( below ) {
            //qDebug() << "below";
            findStr = item->text(0);
            delete item;
            item = 0;
        } else if ( ui->treeWidget->topLevelItemCount() > 0 ) {
            findStr = item->text(0);
            delete item;
            item = 0;
        }
        if(!findStr.isEmpty())
            m_autocorrectEntries.remove(findStr);
    }
    ui->treeWidget->setSortingEnabled(false);

    emitChanged();
}

void AutoCorrectionWidget::updateAddRemoveButton()
{
    QList<QTreeWidgetItem *> listItems = ui->treeWidget->selectedItems ();
    ui->removeButton->setEnabled(!listItems.isEmpty());
}

void AutoCorrectionWidget::enableAddRemoveButton()
{
    const QString find = ui->find->text();
    const QString replace = ui->replace->text();

    QTreeWidgetItem *item = 0;
    if (m_autocorrectEntries.contains(find)) {
        item = ui->treeWidget->findItems(find, Qt::MatchCaseSensitive).first();
    }

    bool enable = false;
    if ( find.isEmpty() || replace.isEmpty()) {// disable if no text in find/replace
        enable = !(find.isEmpty() || replace.isEmpty());
    } else if (item && find == item->text(0)) {
        // We disable add / remove button if no text for the replacement
        enable = !item->text(1).isEmpty();
        ui->addButton->setText(i18n("&Modify"));
    } else if (!item || !item->text(1).isEmpty()) {
        enable = true;
        ui->addButton->setText(i18n("&Add"));
    }

    if (item && replace == item->text(1))
        ui->addButton->setEnabled(false);
    else
        ui->addButton->setEnabled(enable);
    ui->removeButton->setEnabled(enable);

}

void AutoCorrectionWidget::setFindReplaceText(QTreeWidgetItem*item ,int column)
{
    Q_UNUSED(column);
    ui->find->setText(item->text(0));
    ui->replace->setText(item->text(1));
}


void AutoCorrectionWidget::abbreviationChanged(const QString &text)
{
    ui->add1->setEnabled(!text.isEmpty());
}

void AutoCorrectionWidget::twoUpperLetterChanged(const QString &text)
{
    ui->add2->setEnabled(!text.isEmpty());
}

void AutoCorrectionWidget::addAbbreviationEntry()
{
    const QString text = ui->abbreviation->text();
    if (text.isEmpty())
        return;
    if (!m_upperCaseExceptions.contains(text)) {
        m_upperCaseExceptions.insert(text);
        ui->abbreviationList->addItem(text);
    }
    ui->abbreviation->clear();
    slotEnableDisableAbreviationList();
    emitChanged();
}

void AutoCorrectionWidget::removeAbbreviationEntry()
{
    QList<QListWidgetItem *> listItem = ui->abbreviationList->selectedItems ();
    if (listItem.isEmpty())
        return;
    Q_FOREACH( QListWidgetItem *item, listItem ) {
        m_upperCaseExceptions.remove(item->text());
        delete item;
    }
    slotEnableDisableAbreviationList();
    emitChanged();
}

void AutoCorrectionWidget::addTwoUpperLetterEntry()
{
    const QString text = ui->twoUpperLetter->text();
    if (text.isEmpty())
        return;
    if (!m_twoUpperLetterExceptions.contains(text)) {
        m_twoUpperLetterExceptions.insert(text);
        ui->twoUpperLetterList->addItem(text);
        emitChanged();
    }
    slotEnableDisableTwoUpperEntry();
    ui->twoUpperLetter->clear();

}

void AutoCorrectionWidget::removeTwoUpperLetterEntry()
{
    QList<QListWidgetItem *> listItem = ui->twoUpperLetterList->selectedItems ();
    if (listItem.isEmpty())
        return;
    Q_FOREACH( QListWidgetItem *item, listItem ) {
        m_twoUpperLetterExceptions.remove(item->text());
        delete item;
    }
    slotEnableDisableTwoUpperEntry();
    emitChanged();
}


void AutoCorrectionWidget::slotEnableDisableAbreviationList()
{
    const bool enable = (!ui->abbreviationList->selectedItems ().isEmpty());
    ui->add1->setEnabled(!ui->abbreviation->text().isEmpty());
    ui->remove1->setEnabled(enable);
}

void AutoCorrectionWidget::slotEnableDisableTwoUpperEntry()
{
    const bool enable = (!ui->twoUpperLetterList->selectedItems ().isEmpty());
    ui->add2->setEnabled( !ui->twoUpperLetter->text().isEmpty());
    ui->remove2->setEnabled(enable);
}

void AutoCorrectionWidget::slotImportAutoCorrection(QAction* act)
{
    if ( act ) {
        AutoCorrectionWidget::ImportFileType type = act->data().value<AutoCorrectionWidget::ImportFileType>();
        QString title;
        QString filter;
        switch (type) {
        case AutoCorrectionWidget::LibreOffice:
            title = i18n("Import LibreOffice Autocorrection");
            filter = QLatin1String("*.dat");
            break;
        case AutoCorrectionWidget::KMail:
            title = i18n("Import KMail Autocorrection");
            filter = QLatin1String("*.xml");
            break;
        }
        const QString fileName = QFileDialog::getOpenFileName(this, title ,  QString(), filter);
        if ( !fileName.isEmpty() ) {
            PimCommon::ImportAbstractAutocorrection *importAutoCorrection = 0;
            switch(type) {
            case AutoCorrectionWidget::LibreOffice:
                importAutoCorrection = new PimCommon::ImportLibreOfficeAutocorrection(this);
                break;
            case AutoCorrectionWidget::KMail:
                importAutoCorrection = new PimCommon::ImportKMailAutocorrection(this);
                break;
            default:
                return;
            }
            if (importAutoCorrection->import(fileName,ImportAbstractAutocorrection::All)) {
                m_autocorrectEntries = importAutoCorrection->autocorrectEntries();
                addAutoCorrectEntries();

                enableAdvAutocorrection(ui->advancedAutocorrection->isChecked());

                m_upperCaseExceptions = importAutoCorrection->upperCaseExceptions();
                m_twoUpperLetterExceptions = importAutoCorrection->twoUpperLetterExceptions();

                ui->twoUpperLetterList->clear();
                ui->twoUpperLetterList->addItems(m_twoUpperLetterExceptions.toList());

                ui->abbreviationList->clear();
                ui->abbreviationList->addItems(m_upperCaseExceptions.toList());
            }
            delete importAutoCorrection;
        }
    }
}

void AutoCorrectionWidget::setLanguage(const QString &lang)
{
    mAutoCorrection->setLanguage(lang);
    loadAutoCorrectionAndException();
    mWasChanged = false;
}

void AutoCorrectionWidget::changeLanguage(int index)
{
    if (index == -1)
        return;
    if (mWasChanged) {
        const int rc = KMessageBox::warningYesNo( this,i18n("Language was changed, do you want to save config for previous language?"),i18n( "Save config" ) );
        if ( rc == KMessageBox::Yes ) {
            writeConfig();
        }
    }
    const QString lang = ui->autocorrectionLanguage->itemData (index).toString();
    mAutoCorrection->setLanguage(lang);
    loadAutoCorrectionAndException();
    mWasChanged = false;
}

void AutoCorrectionWidget::emitChanged()
{
    mWasChanged = true;
    Q_EMIT changed();
}

void AutoCorrectionWidget::loadGlobalAutoCorrectionAndException()
{
    const QString lang = ui->autocorrectionLanguage->itemData (ui->autocorrectionLanguage->currentIndex()).toString();
    mAutoCorrection->setLanguage(lang,true);
    loadAutoCorrectionAndException();
    mWasChanged = true;
    Q_EMIT changed();
}

void AutoCorrectionWidget::slotExportAutoCorrection()
{
    const KUrl saveUrl= KFileDialog::getSaveUrl(QDir::homePath(), QString(), this, i18n( "Export Autocorrection File" ) );
    if ( saveUrl.isEmpty() ) {
        return;
    }
    mAutoCorrection->writeAutoCorrectionXmlFile(saveUrl.toLocalFile());
}

