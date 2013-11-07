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

#include "composerautocorrectionwidget.h"
#include "composerautocorrection.h"
#include "ui_composerautocorrectionwidget.h"
#include "import/importlibreofficeautocorrection.h"
#include "import/importkmailautocorrection.h"
#include "import/importabstractautocorrection.h"

#include "settings/messagecomposersettings.h"
#include <kpimtextedit/selectspecialchar.h>

#include <KFileDialog>
#include <KMessageBox>

#include <QTreeWidgetItem>
#include <QMenu>

using namespace MessageComposer;

Q_DECLARE_METATYPE(ComposerAutoCorrectionWidget::ImportFileType)

ComposerAutoCorrectionWidget::ComposerAutoCorrectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ComposerAutoCorrectionWidget),
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
    connect(ui->typographicSingleQuotes, SIGNAL(clicked(bool)), this, SLOT(enableSingleQuotes(bool)));
    connect(ui->typographicDoubleQuotes, SIGNAL(clicked(bool)), this, SLOT(enableDoubleQuotes(bool)));
    connect(ui->autoSuperScript,SIGNAL(clicked()),SIGNAL(changed()));
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
    connect(ui->treeWidget,SIGNAL(deleteSelectedItems()),SLOT(removeAutocorrectEntry()));
    connect(ui->treeWidget,SIGNAL(itemSelectionChanged()),SLOT(updateAddRemoveButton()));
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
    connect(ui->abbreviationList,SIGNAL(itemSelectionChanged()),SLOT(slotEnableDisableAbreviationList()));
    connect(ui->abbreviationList,SIGNAL(deleteSelectedItems()),SLOT(removeAbbreviationEntry()));
    connect(ui->twoUpperLetterList,SIGNAL(itemSelectionChanged()),SLOT(slotEnableDisableTwoUpperEntry()));
    connect(ui->twoUpperLetterList,SIGNAL(deleteSelectedItems()),SLOT(removeTwoUpperLetterEntry()));
    connect(ui->autocorrectionLanguage,SIGNAL(activated(int)),SLOT(changeLanguage(int)));
    connect(ui->addNonBreakingSpaceInFrench,SIGNAL(clicked()),SIGNAL(changed()));
    slotEnableDisableAbreviationList();
    slotEnableDisableTwoUpperEntry();

    QMenu *menu = new QMenu();
    ui->importAutoCorrection->setMenu( menu );

    QAction *act = new QAction( i18n( "LibreOffice Autocorrection" ), this );
    act->setData( QVariant::fromValue( ComposerAutoCorrectionWidget::LibreOffice ) );
    menu->addAction( act );

    act = new QAction( i18n( "KMail/Calligra Autocorrection" ), this );
    act->setData( QVariant::fromValue( ComposerAutoCorrectionWidget::KMail ) );
    menu->addAction( act );

    connect( menu, SIGNAL(triggered(QAction*)), SLOT(slotImportAutoCorrection(QAction*)) );

    connect(ui->exportAutoCorrection,SIGNAL(clicked()),SLOT(slotExportAutoCorrection()));
}

ComposerAutoCorrectionWidget::~ComposerAutoCorrectionWidget()
{
    delete ui;
}

void ComposerAutoCorrectionWidget::setAutoCorrection(ComposerAutoCorrection * autoCorrect)
{
    mAutoCorrection = autoCorrect;
    setLanguage(ui->autocorrectionLanguage->language());
}

void ComposerAutoCorrectionWidget::loadConfig()
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

void ComposerAutoCorrectionWidget::loadAutoCorrectionAndException()
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

void ComposerAutoCorrectionWidget::addAutoCorrectEntries()
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

void ComposerAutoCorrectionWidget::writeConfig()
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

void ComposerAutoCorrectionWidget::resetToDefault()
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

void ComposerAutoCorrectionWidget::enableSingleQuotes(bool state)
{
    ui->singleQuote1->setEnabled(state);
    ui->singleQuote2->setEnabled(state);
    ui->singleDefault->setEnabled(state);
}

void ComposerAutoCorrectionWidget::enableDoubleQuotes(bool state)
{
    ui->doubleQuote1->setEnabled(state);
    ui->doubleQuote2->setEnabled(state);
    ui->doubleDefault->setEnabled(state);
}

void ComposerAutoCorrectionWidget::selectSingleQuoteCharOpen()
{
    KPIMTextEdit::SelectSpecialChar dlg(this);
    dlg.setCurrentChar(m_singleQuotes.begin);
    dlg.showSelectButton(false);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        m_singleQuotes.begin = dlg.currentChar();
        ui->singleQuote1->setText(m_singleQuotes.begin);
        emitChanged();
    }
}

void ComposerAutoCorrectionWidget::selectSingleQuoteCharClose()
{
    KPIMTextEdit::SelectSpecialChar dlg(this);
    dlg.showSelectButton(false);
    dlg.setCurrentChar(m_singleQuotes.end);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        m_singleQuotes.end = dlg.currentChar();
        ui->singleQuote2->setText(m_singleQuotes.end);
        emitChanged();
    }
}

void ComposerAutoCorrectionWidget::setDefaultSingleQuotes()
{
    m_singleQuotes = mAutoCorrection->typographicDefaultSingleQuotes();
    ui->singleQuote1->setText(m_singleQuotes.begin);
    ui->singleQuote2->setText(m_singleQuotes.end);
    emitChanged();
}

void ComposerAutoCorrectionWidget::selectDoubleQuoteCharOpen()
{
    KPIMTextEdit::SelectSpecialChar dlg(this);
    dlg.showSelectButton(false);
    dlg.setCurrentChar(m_doubleQuotes.begin);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        m_doubleQuotes.begin = dlg.currentChar();
        ui->doubleQuote1->setText(m_doubleQuotes.begin);
        emitChanged();
    }
}

void ComposerAutoCorrectionWidget::selectDoubleQuoteCharClose()
{
    KPIMTextEdit::SelectSpecialChar dlg(this);
    dlg.showSelectButton(false);
    dlg.setCurrentChar(m_doubleQuotes.end);
    dlg.autoInsertChar();
    if (dlg.exec()) {
        m_doubleQuotes.end = dlg.currentChar();
        ui->doubleQuote2->setText(m_doubleQuotes.end);
        emitChanged();
    }
}

void ComposerAutoCorrectionWidget::setDefaultDoubleQuotes()
{
    m_doubleQuotes = mAutoCorrection->typographicDefaultDoubleQuotes();
    ui->doubleQuote1->setText(m_doubleQuotes.begin);
    ui->doubleQuote2->setText(m_doubleQuotes.end);
    emitChanged();
}

void ComposerAutoCorrectionWidget::enableAdvAutocorrection(bool state)
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


void ComposerAutoCorrectionWidget::addAutocorrectEntry()
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

void ComposerAutoCorrectionWidget::removeAutocorrectEntry()
{
    QList<QTreeWidgetItem *> listItems = ui->treeWidget->selectedItems ();
    if (listItems.isEmpty())
        return;
    Q_FOREACH (QTreeWidgetItem *item, listItems) {
        QTreeWidgetItem *below = ui->treeWidget->itemBelow( item );

        QString findStr;
        if ( below ) {
            //kDebug() << "below";
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

void ComposerAutoCorrectionWidget::updateAddRemoveButton()
{
    QList<QTreeWidgetItem *> listItems = ui->treeWidget->selectedItems ();
    ui->removeButton->setEnabled(!listItems.isEmpty());
}

void ComposerAutoCorrectionWidget::enableAddRemoveButton()
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

void ComposerAutoCorrectionWidget::setFindReplaceText(QTreeWidgetItem*item ,int column)
{
    Q_UNUSED(column);
    ui->find->setText(item->text(0));
    ui->replace->setText(item->text(1));
}


void ComposerAutoCorrectionWidget::abbreviationChanged(const QString &text)
{
    ui->add1->setEnabled(!text.isEmpty());
}

void ComposerAutoCorrectionWidget::twoUpperLetterChanged(const QString &text)
{
    ui->add2->setEnabled(!text.isEmpty());
}

void ComposerAutoCorrectionWidget::addAbbreviationEntry()
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

void ComposerAutoCorrectionWidget::removeAbbreviationEntry()
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

void ComposerAutoCorrectionWidget::addTwoUpperLetterEntry()
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

void ComposerAutoCorrectionWidget::removeTwoUpperLetterEntry()
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


void ComposerAutoCorrectionWidget::slotEnableDisableAbreviationList()
{
    const bool enable = (!ui->abbreviationList->selectedItems ().isEmpty());
    ui->add1->setEnabled(!ui->abbreviation->text().isEmpty());
    ui->remove1->setEnabled(enable);
}

void ComposerAutoCorrectionWidget::slotEnableDisableTwoUpperEntry()
{
    const bool enable = (!ui->twoUpperLetterList->selectedItems ().isEmpty());
    ui->add2->setEnabled( !ui->twoUpperLetter->text().isEmpty());
    ui->remove2->setEnabled(enable);
}

void ComposerAutoCorrectionWidget::slotImportAutoCorrection(QAction* act)
{
    if ( act ) {
        ComposerAutoCorrectionWidget::ImportFileType type = act->data().value<ComposerAutoCorrectionWidget::ImportFileType>();
        QString title;
        QString filter;
        switch (type) {
        case ComposerAutoCorrectionWidget::LibreOffice:
            title = i18n("Import LibreOffice Autocorrection");
            filter = QLatin1String("*.dat");
            break;
        case ComposerAutoCorrectionWidget::KMail:
            title = i18n("Import KMail Autocorrection");
            filter = QLatin1String("*.xml");
            break;
        }
        const QString fileName = KFileDialog::getOpenFileName( QString(), filter, this, title );
        if ( !fileName.isEmpty() ) {
            MessageComposer::ImportAbstractAutocorrection *importAutoCorrection = 0;
            switch(type) {
            case ComposerAutoCorrectionWidget::LibreOffice:
                importAutoCorrection = new MessageComposer::ImportLibreOfficeAutocorrection(this);
                break;
            case ComposerAutoCorrectionWidget::KMail:
                importAutoCorrection = new MessageComposer::ImportKMailAutocorrection(this);
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

void ComposerAutoCorrectionWidget::setLanguage(const QString &lang)
{
    mAutoCorrection->setLanguage(lang);
    loadAutoCorrectionAndException();
    mWasChanged = false;
}

void ComposerAutoCorrectionWidget::changeLanguage(int index)
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

void ComposerAutoCorrectionWidget::emitChanged()
{
    mWasChanged = true;
    Q_EMIT changed();
}

void ComposerAutoCorrectionWidget::loadGlobalAutoCorrectionAndException()
{
    const QString lang = ui->autocorrectionLanguage->itemData (ui->autocorrectionLanguage->currentIndex()).toString();
    mAutoCorrection->setLanguage(lang,true);
    loadAutoCorrectionAndException();
    mWasChanged = true;
    Q_EMIT changed();
}

void ComposerAutoCorrectionWidget::slotExportAutoCorrection()
{
    const KUrl saveUrl= KFileDialog::getSaveUrl(QDir::homePath(), QString(), this, i18n( "Export Autocorrection File" ) );
    if ( saveUrl.isEmpty() ) {
        return;
    }
    mAutoCorrection->writeAutoCorrectionXmlFile(saveUrl.toLocalFile());
}

