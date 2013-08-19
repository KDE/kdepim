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

#include "imagescalingwidget.h"
#include "ui_imagescalingwidget.h"
#include "settings/messagecomposersettings.h"

#include <KComboBox>
#include <KLocale>
#include <KMessageBox>

#include <QImageWriter>
#include <QWhatsThis>

using namespace MessageComposer;

ImageScalingWidget::ImageScalingWidget(QWidget *parent)
    :QWidget(parent),
      ui(new Ui::ImageScalingWidget),
      mWasChanged(false)
{
    ui->setupUi(this);
    initComboBox(ui->CBMaximumWidth);
    initComboBox(ui->CBMaximumHeight);
    initComboBox(ui->CBMinimumWidth);
    initComboBox(ui->CBMinimumHeight);

    initWriteImageFormat();
    connect(ui->enabledAutoResize,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->KeepImageRatio,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->AskBeforeResizing,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->EnlargeImageToMinimum,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->ReduceImageToMaximum,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->customMaximumWidth,SIGNAL(valueChanged(int)),SIGNAL(changed()));
    connect(ui->customMaximumHeight,SIGNAL(valueChanged(int)),SIGNAL(changed()));
    connect(ui->customMinimumWidth,SIGNAL(valueChanged(int)),SIGNAL(changed()));
    connect(ui->customMinimumHeight,SIGNAL(valueChanged(int)),SIGNAL(changed()));
    connect(ui->skipImageSizeLower,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->imageSize,SIGNAL(valueChanged(int)),SIGNAL(changed()));
    connect(ui->pattern,SIGNAL(textChanged(QString)),SIGNAL(changed()));
    connect(ui->CBMaximumWidth,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
    connect(ui->CBMaximumHeight,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
    connect(ui->CBMinimumWidth,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
    connect(ui->CBMinimumHeight,SIGNAL(currentIndexChanged(int)),SLOT(slotComboboxChanged(int)));
    connect(ui->WriteToImageFormat,SIGNAL(activated(int)),SIGNAL(changed()));
    connect(ui->renameResizedImage,SIGNAL(clicked()),SIGNAL(changed()));
    connect(ui->renameResizedImage,SIGNAL(clicked(bool)),ui->renameResizedImagePattern,SLOT(setEnabled(bool)));
    connect(ui->renameResizedImagePattern,SIGNAL(textChanged(QString)),SIGNAL(changed()));

    connect(ui->resizeEmailsPattern,SIGNAL(textChanged(QString)),SIGNAL(changed()));
    connect(ui->doNotResizePattern,SIGNAL(textChanged(QString)),SIGNAL(changed()));
    connect(ui->resizeImageWithFormatsType,SIGNAL(textChanged(QString)),SIGNAL(changed()));
    connect(ui->resizeImageWithFormats,SIGNAL(clicked(bool)),SIGNAL(changed()));
    connect(ui->resizeImageWithFormats,SIGNAL(clicked(bool)),ui->resizeImageWithFormatsType,SLOT(setEnabled(bool)));
    ui->resizeImageWithFormatsType->setEnabled(false);

    ui->pattern->setEnabled(false);
    mSourceFilenameFilterGroup = new QButtonGroup(ui->filterSourceGroupBox);
    connect( mSourceFilenameFilterGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotSourceFilterClicked(int)) );
    mSourceFilenameFilterGroup->addButton( ui->notFilterFilename, MessageComposer::MessageComposerSettings::EnumFilterSourceType::NoFilter );
    mSourceFilenameFilterGroup->addButton( ui->includeFilesWithPattern, MessageComposer::MessageComposerSettings::EnumFilterSourceType::IncludeFilesWithPattern );
    mSourceFilenameFilterGroup->addButton( ui->excludeFilesWithPattern, MessageComposer::MessageComposerSettings::EnumFilterSourceType::ExcludeFilesWithPattern );

    mRecipientFilterGroup = new QButtonGroup(ui->tab_4);
    connect( mRecipientFilterGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotRecipientFilterClicked(int)) );
    ui->doNotResizePattern->setEnabled(false);
    ui->resizeEmailsPattern->setEnabled(false);
    mRecipientFilterGroup->addButton(ui->doNotFilterRecipients,MessageComposer::MessageComposerSettings::EnumFilterRecipientType::NoFilter );
    mRecipientFilterGroup->addButton(ui->resizeEachEmails,MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeEachEmailsContainsPattern );
    mRecipientFilterGroup->addButton(ui->resizeOneEmails,MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeOneEmailContainsPattern );
    mRecipientFilterGroup->addButton(ui->doNotResizeEachEmails,MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeEachEmailsContainsPattern );
    mRecipientFilterGroup->addButton(ui->doNotResizeOneEmails,MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeOneEmailContainsPattern );

    ui->help->setText( i18n( "<a href=\"whatsthis\">How does this work?</a>" ) );
    connect( ui->help, SIGNAL(linkActivated(QString)),SLOT(slotHelpLinkClicked(QString)) );
}

ImageScalingWidget::~ImageScalingWidget()
{
    delete ui;
}

void ImageScalingWidget::slotHelpLinkClicked(const QString&)
{
    const QString help =
            i18n( "<qt>"
                  "<p>Here you can define image filename. "
                  "You can use:</p>"
                  "<ul>"
                  "<li>%t set current time</li>"
                  "<li>%d set current date</li>"
                  "<li>%n original filename</li>"
                  "<li>%e original extension</li>"
                  "<li>%x new extension</li>"
                  "</ul>"
                  "</qt>" );

    QWhatsThis::showText( QCursor::pos(), help );
}

void ImageScalingWidget::slotSourceFilterClicked(int button)
{
    ui->pattern->setEnabled(button != 0);
    Q_EMIT changed();
}

void ImageScalingWidget::slotRecipientFilterClicked(int button)
{
    ui->resizeEmailsPattern->setEnabled( (button == MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeEachEmailsContainsPattern) ||
                                         (button == MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeOneEmailContainsPattern) );
    ui->doNotResizePattern->setEnabled((button == MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeEachEmailsContainsPattern) ||
                                       (button == MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeOneEmailContainsPattern) );
    Q_EMIT changed();
}

void ImageScalingWidget::slotComboboxChanged(int index)
{
    KComboBox* combo = qobject_cast< KComboBox* >( sender() );
    if (combo) {
        const bool isCustom = combo->itemData(index) == -1;
        if(combo == ui->CBMaximumWidth) {
            ui->customMaximumWidth->setEnabled(isCustom);
        } else if(combo == ui->CBMaximumHeight) {
            ui->customMaximumHeight->setEnabled(isCustom);
        } else if(combo == ui->CBMinimumWidth) {
            ui->customMinimumWidth->setEnabled(isCustom);
        } else if(combo == ui->CBMinimumHeight) {
            ui->customMinimumHeight->setEnabled(isCustom);
        }
        Q_EMIT changed();
    }
}

void ImageScalingWidget::initComboBox(KComboBox *combo)
{
    QList<int> size;
    size <<240 <<320 <<512 <<640 <<800 <<1024 <<1600 <<2048;
    Q_FOREACH(int val, size) {
        combo->addItem(QString::number(val), val);
    }
    combo->addItem(i18n("Custom"), -1);
}

void ImageScalingWidget::initWriteImageFormat()
{
    /* Too many format :)
    QList<QByteArray> listWriteFormat = QImageWriter::supportedImageFormats();
    Q_FOREACH(const QByteArray& format, listWriteFormat) {
        ui->WriteToImageFormat->addItem(QString::fromLatin1(format));
    }
    */
    //known by several mailer.
    ui->WriteToImageFormat->addItem(QString::fromLatin1("JPG"));
    ui->WriteToImageFormat->addItem(QString::fromLatin1("PNG"));
}

void ImageScalingWidget::updateSettings()
{
    ui->enabledAutoResize->setChecked(MessageComposer::MessageComposerSettings::self()->autoResizeImageEnabled());
    ui->KeepImageRatio->setChecked(MessageComposer::MessageComposerSettings::self()->keepImageRatio());
    ui->AskBeforeResizing->setChecked(MessageComposer::MessageComposerSettings::self()->askBeforeResizing());
    ui->EnlargeImageToMinimum->setChecked(MessageComposer::MessageComposerSettings::self()->enlargeImageToMinimum());
    ui->ReduceImageToMaximum->setChecked(MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum());
    ui->skipImageSizeLower->setChecked(MessageComposer::MessageComposerSettings::self()->skipImageLowerSizeEnabled());
    ui->imageSize->setValue(MessageComposer::MessageComposerSettings::self()->skipImageLowerSize());

    ui->customMaximumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumWidth());
    ui->customMaximumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumHeight());
    ui->customMinimumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumWidth());
    ui->customMinimumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumHeight());

    int index = qMax(0, ui->CBMaximumWidth->findData(MessageComposer::MessageComposerSettings::self()->maximumWidth()));
    ui->CBMaximumWidth->setCurrentIndex(index);
    ui->customMaximumWidth->setEnabled(ui->CBMaximumWidth->itemData(index) == -1);

    index = qMax(0, ui->CBMaximumHeight->findData(MessageComposer::MessageComposerSettings::self()->maximumHeight()));
    ui->CBMaximumHeight->setCurrentIndex(index);
    ui->customMaximumHeight->setEnabled(ui->CBMaximumHeight->itemData(index) == -1);

    index = qMax(0, ui->CBMinimumWidth->findData(MessageComposer::MessageComposerSettings::self()->minimumWidth()));
    ui->CBMinimumWidth->setCurrentIndex(index);
    ui->customMinimumWidth->setEnabled(ui->CBMinimumWidth->itemData(index) == -1);

    index = qMax(0, ui->CBMinimumHeight->findData(MessageComposer::MessageComposerSettings::self()->minimumHeight()));
    ui->CBMinimumHeight->setCurrentIndex(index);
    ui->customMinimumHeight->setEnabled(ui->CBMinimumHeight->itemData(index) == -1);

    index = ui->WriteToImageFormat->findData(MessageComposer::MessageComposerSettings::self()->writeFormat());
    if(index == -1) {
        ui->WriteToImageFormat->setCurrentIndex(0);
    } else {
        ui->WriteToImageFormat->setCurrentIndex(index);
    }
    ui->pattern->setText(MessageComposer::MessageComposerSettings::self()->filterSourcePattern());

    ui->renameResizedImage->setChecked(MessageComposer::MessageComposerSettings::self()->renameResizedImages());

    ui->renameResizedImagePattern->setText(MessageComposer::MessageComposerSettings::self()->renameResizedImagesPattern());
    ui->renameResizedImagePattern->setEnabled(ui->renameResizedImage->isChecked());

    ui->doNotResizePattern->setText(MessageComposer::MessageComposerSettings::self()->doNotResizeEmailsPattern());
    ui->resizeEmailsPattern->setText(MessageComposer::MessageComposerSettings::self()->resizeEmailsPattern());

    ui->resizeImageWithFormats->setChecked(MessageComposer::MessageComposerSettings::self()->resizeImagesWithFormats());
    ui->resizeImageWithFormatsType->setFormat(MessageComposer::MessageComposerSettings::self()->resizeImagesWithFormatsType());
    ui->resizeImageWithFormatsType->setEnabled(ui->resizeImageWithFormats->isChecked());


    updateFilterSourceTypeSettings();
    updateEmailsFilterTypeSettings();
}

void ImageScalingWidget::loadConfig()
{
    updateSettings();
    mWasChanged = false;
}

void ImageScalingWidget::updateFilterSourceTypeSettings()
{
    switch(MessageComposer::MessageComposerSettings::self()->filterSourceType()) {
    case MessageComposer::MessageComposerSettings::EnumFilterSourceType::NoFilter:
        ui->notFilterFilename->setChecked(true);
        ui->pattern->setEnabled(false);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterSourceType::IncludeFilesWithPattern:
        ui->includeFilesWithPattern->setChecked(true);
        ui->pattern->setEnabled(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterSourceType::ExcludeFilesWithPattern:
        ui->excludeFilesWithPattern->setChecked(true);
        ui->pattern->setEnabled(true);
        break;
    }
}

void ImageScalingWidget::updateEmailsFilterTypeSettings()
{
    ui->doNotResizePattern->setEnabled(false);
    ui->resizeEmailsPattern->setEnabled(false);

    switch(MessageComposer::MessageComposerSettings::self()->filterRecipientType()) {
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::NoFilter:
        ui->doNotFilterRecipients->setChecked(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeEachEmailsContainsPattern:
        ui->resizeEachEmails->setChecked(true);
        ui->resizeEmailsPattern->setEnabled(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeOneEmailContainsPattern:
        ui->resizeOneEmails->setChecked(true);
        ui->resizeEmailsPattern->setEnabled(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeEachEmailsContainsPattern:
        ui->doNotResizeEachEmails->setChecked(true);
        ui->doNotResizePattern->setEnabled(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeOneEmailContainsPattern:
        ui->doNotResizeOneEmails->setChecked(true);
        ui->doNotResizePattern->setEnabled(false);
        break;
    }
}

void ImageScalingWidget::writeConfig()
{
    if (ui->EnlargeImageToMinimum->isChecked() && ui->ReduceImageToMaximum->isChecked()) {
        if ((ui->customMinimumWidth->value()>=ui->customMaximumWidth->value()) ||
                (ui->customMinimumHeight->value()>=ui->customMaximumHeight->value())) {
            KMessageBox::error(this, i18n("Please verify minimum and maximum values."), i18n("Error in minimum Maximum value"));
            return;
        }
    }
    MessageComposer::MessageComposerSettings::self()->setAutoResizeImageEnabled(ui->enabledAutoResize->isChecked());
    MessageComposer::MessageComposerSettings::self()->setKeepImageRatio(ui->KeepImageRatio->isChecked());
    MessageComposer::MessageComposerSettings::self()->setAskBeforeResizing(ui->AskBeforeResizing->isChecked());
    MessageComposer::MessageComposerSettings::self()->setEnlargeImageToMinimum(ui->EnlargeImageToMinimum->isChecked());
    MessageComposer::MessageComposerSettings::self()->setReduceImageToMaximum(ui->ReduceImageToMaximum->isChecked());

    MessageComposer::MessageComposerSettings::self()->setCustomMaximumWidth(ui->customMaximumWidth->value());
    MessageComposer::MessageComposerSettings::self()->setCustomMaximumHeight(ui->customMaximumHeight->value());
    MessageComposer::MessageComposerSettings::self()->setCustomMinimumWidth(ui->customMinimumWidth->value());
    MessageComposer::MessageComposerSettings::self()->setCustomMinimumHeight(ui->customMinimumHeight->value());

    MessageComposer::MessageComposerSettings::self()->setMaximumWidth(ui->CBMaximumWidth->itemData(ui->CBMaximumWidth->currentIndex()).toInt());
    MessageComposer::MessageComposerSettings::self()->setMaximumHeight(ui->CBMaximumHeight->itemData(ui->CBMaximumHeight->currentIndex()).toInt());
    MessageComposer::MessageComposerSettings::self()->setMinimumWidth(ui->CBMinimumWidth->itemData(ui->CBMinimumWidth->currentIndex()).toInt());
    MessageComposer::MessageComposerSettings::self()->setMinimumHeight(ui->CBMinimumHeight->itemData(ui->CBMinimumHeight->currentIndex()).toInt());

    MessageComposer::MessageComposerSettings::self()->setWriteFormat(ui->WriteToImageFormat->currentText());
    MessageComposer::MessageComposerSettings::self()->setSkipImageLowerSizeEnabled(ui->skipImageSizeLower->isChecked());
    MessageComposer::MessageComposerSettings::self()->setSkipImageLowerSize(ui->imageSize->value());

    MessageComposer::MessageComposerSettings::self()->setFilterSourcePattern(ui->pattern->text());

    MessageComposer::MessageComposerSettings::self()->setFilterSourceType(mSourceFilenameFilterGroup->checkedId());

    MessageComposer::MessageComposerSettings::self()->setRenameResizedImages(ui->renameResizedImage->isChecked());

    MessageComposer::MessageComposerSettings::self()->setRenameResizedImagesPattern(ui->renameResizedImagePattern->text());

    MessageComposer::MessageComposerSettings::self()->setDoNotResizeEmailsPattern(ui->doNotResizePattern->text());
    MessageComposer::MessageComposerSettings::self()->setResizeEmailsPattern(ui->resizeEmailsPattern->text());
    MessageComposer::MessageComposerSettings::self()->setFilterRecipientType(mRecipientFilterGroup->checkedId());

    MessageComposer::MessageComposerSettings::self()->setResizeImagesWithFormats(ui->resizeImageWithFormats->isChecked());
    MessageComposer::MessageComposerSettings::self()->setResizeImagesWithFormatsType(ui->resizeImageWithFormatsType->format());
    mWasChanged = false;
}

void ImageScalingWidget::resetToDefault()
{
    const bool bUseDefaults = MessageComposer::MessageComposerSettings::self()->useDefaults( true );
    updateSettings();
    MessageComposer::MessageComposerSettings::self()->useDefaults( bUseDefaults );
}

#include "imagescalingwidget.moc"
