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

#include "imagescalingwidget.h"
#include "ui_imagescalingwidget.h"
#include "settings/messagecomposersettings.h"

#include <KComboBox>
#include <KLocalizedString>
#include <KMessageBox>

#include <QImageWriter>
#include <QWhatsThis>

using namespace MessageComposer;
class MessageComposer::ImageScalingWidgetPrivate
{
public:
    ImageScalingWidgetPrivate()
        : ui(new Ui::ImageScalingWidget),
          mSourceFilenameFilterGroup(Q_NULLPTR),
          mRecipientFilterGroup(Q_NULLPTR),
          mWasChanged(false)
    {
    }
    ~ImageScalingWidgetPrivate()
    {
        delete ui;
    }

    Ui::ImageScalingWidget *ui;
    QButtonGroup *mSourceFilenameFilterGroup;
    QButtonGroup *mRecipientFilterGroup;
    bool mWasChanged;
};

ImageScalingWidget::ImageScalingWidget(QWidget *parent)
    : QWidget(parent),
      d(new MessageComposer::ImageScalingWidgetPrivate)
{
    d->ui->setupUi(this);
    initComboBox(d->ui->CBMaximumWidth);
    initComboBox(d->ui->CBMaximumHeight);
    initComboBox(d->ui->CBMinimumWidth);
    initComboBox(d->ui->CBMinimumHeight);

    initWriteImageFormat();
    connect(d->ui->enabledAutoResize, &QCheckBox::clicked, this, &ImageScalingWidget::changed);
    connect(d->ui->KeepImageRatio, &QCheckBox::clicked, this, &ImageScalingWidget::changed);
    connect(d->ui->AskBeforeResizing, &QCheckBox::clicked, this, &ImageScalingWidget::changed);
    connect(d->ui->EnlargeImageToMinimum, &QCheckBox::clicked, this, &ImageScalingWidget::changed);
    connect(d->ui->ReduceImageToMaximum, &QCheckBox::clicked, this, &ImageScalingWidget::changed);
    connect(d->ui->customMaximumWidth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ImageScalingWidget::changed);
    connect(d->ui->customMaximumHeight, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ImageScalingWidget::changed);
    connect(d->ui->customMinimumWidth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ImageScalingWidget::changed);
    connect(d->ui->customMinimumHeight, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ImageScalingWidget::changed);
    connect(d->ui->skipImageSizeLower, &QCheckBox::clicked, this, &ImageScalingWidget::changed);
    connect(d->ui->imageSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ImageScalingWidget::changed);
    connect(d->ui->pattern, &KLineEdit::textChanged, this, &ImageScalingWidget::changed);
    connect(d->ui->CBMaximumWidth, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ImageScalingWidget::slotComboboxChanged);
    connect(d->ui->CBMaximumHeight, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ImageScalingWidget::slotComboboxChanged);
    connect(d->ui->CBMinimumWidth, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ImageScalingWidget::slotComboboxChanged);
    connect(d->ui->CBMinimumHeight, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ImageScalingWidget::slotComboboxChanged);
    connect(d->ui->WriteToImageFormat, static_cast<void (KComboBox::*)(int)>(&KComboBox::activated), this, &ImageScalingWidget::changed);
    connect(d->ui->renameResizedImage, &QCheckBox::clicked, this, &ImageScalingWidget::changed);
    connect(d->ui->renameResizedImage, &QCheckBox::clicked, d->ui->renameResizedImagePattern, &KLineEdit::setEnabled);
    connect(d->ui->renameResizedImagePattern, &KLineEdit::textChanged, this, &ImageScalingWidget::changed);

    connect(d->ui->resizeEmailsPattern, &KLineEdit::textChanged, this, &ImageScalingWidget::changed);
    connect(d->ui->doNotResizePattern, &KLineEdit::textChanged, this, &ImageScalingWidget::changed);
    connect(d->ui->resizeImageWithFormatsType, &MessageComposer::ImageScalingSelectFormat::textChanged, this, &ImageScalingWidget::changed);
    connect(d->ui->resizeImageWithFormats, &QCheckBox::clicked, this, &ImageScalingWidget::changed);
    connect(d->ui->resizeImageWithFormats, &QCheckBox::clicked, d->ui->resizeImageWithFormatsType, &MessageComposer::ImageScalingSelectFormat::setEnabled);
    d->ui->resizeImageWithFormatsType->setEnabled(false);

    d->ui->pattern->setEnabled(false);
    d->mSourceFilenameFilterGroup = new QButtonGroup(d->ui->filterSourceGroupBox);
    connect(d->mSourceFilenameFilterGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ImageScalingWidget::slotSourceFilterClicked);
    d->mSourceFilenameFilterGroup->addButton(d->ui->notFilterFilename, MessageComposer::MessageComposerSettings::EnumFilterSourceType::NoFilter);
    d->mSourceFilenameFilterGroup->addButton(d->ui->includeFilesWithPattern, MessageComposer::MessageComposerSettings::EnumFilterSourceType::IncludeFilesWithPattern);
    d->mSourceFilenameFilterGroup->addButton(d->ui->excludeFilesWithPattern, MessageComposer::MessageComposerSettings::EnumFilterSourceType::ExcludeFilesWithPattern);

    d->mRecipientFilterGroup = new QButtonGroup(d->ui->tab_4);
    connect(d->mRecipientFilterGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ImageScalingWidget::slotRecipientFilterClicked);
    d->ui->doNotResizePattern->setEnabled(false);
    d->ui->resizeEmailsPattern->setEnabled(false);
    d->mRecipientFilterGroup->addButton(d->ui->doNotFilterRecipients, MessageComposer::MessageComposerSettings::EnumFilterRecipientType::NoFilter);
    d->mRecipientFilterGroup->addButton(d->ui->resizeEachEmails, MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeEachEmailsContainsPattern);
    d->mRecipientFilterGroup->addButton(d->ui->resizeOneEmails, MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeOneEmailContainsPattern);
    d->mRecipientFilterGroup->addButton(d->ui->doNotResizeEachEmails, MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeEachEmailsContainsPattern);
    d->mRecipientFilterGroup->addButton(d->ui->doNotResizeOneEmails, MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeOneEmailContainsPattern);

    d->ui->help->setText(i18n("<a href=\"whatsthis\">How does this work?</a>"));
    connect(d->ui->help, &QLabel::linkActivated, this, &ImageScalingWidget::slotHelpLinkClicked);
    d->ui->help->setContextMenuPolicy(Qt::NoContextMenu);
}

ImageScalingWidget::~ImageScalingWidget()
{
    delete d;
}

void ImageScalingWidget::slotHelpLinkClicked(const QString &)
{
    const QString help =
        i18n("<qt>"
             "<p>Here you can define image filename. "
             "You can use:</p>"
             "<ul>"
             "<li>%t set current time</li>"
             "<li>%d set current date</li>"
             "<li>%n original filename</li>"
             "<li>%e original extension</li>"
             "<li>%x new extension</li>"
             "</ul>"
             "</qt>");

    QWhatsThis::showText(QCursor::pos(), help);
}

void ImageScalingWidget::slotSourceFilterClicked(int button)
{
    d->ui->pattern->setEnabled(button != 0);
    Q_EMIT changed();
}

void ImageScalingWidget::slotRecipientFilterClicked(int button)
{
    d->ui->resizeEmailsPattern->setEnabled((button == MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeEachEmailsContainsPattern) ||
                                           (button == MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeOneEmailContainsPattern));
    d->ui->doNotResizePattern->setEnabled((button == MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeEachEmailsContainsPattern) ||
                                          (button == MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeOneEmailContainsPattern));
    Q_EMIT changed();
}

void ImageScalingWidget::slotComboboxChanged(int index)
{
    KComboBox *combo = qobject_cast< KComboBox * >(sender());
    if (combo) {
        const bool isCustom = combo->itemData(index) == -1;
        if (combo == d->ui->CBMaximumWidth) {
            d->ui->customMaximumWidth->setEnabled(isCustom);
        } else if (combo == d->ui->CBMaximumHeight) {
            d->ui->customMaximumHeight->setEnabled(isCustom);
        } else if (combo == d->ui->CBMinimumWidth) {
            d->ui->customMinimumWidth->setEnabled(isCustom);
        } else if (combo == d->ui->CBMinimumHeight) {
            d->ui->customMinimumHeight->setEnabled(isCustom);
        }
        Q_EMIT changed();
    }
}

void ImageScalingWidget::initComboBox(KComboBox *combo)
{
    QList<int> size;
    size << 240 << 320 << 512 << 640 << 800 << 1024 << 1600 << 2048;
    Q_FOREACH (int val, size) {
        combo->addItem(QString::number(val), val);
    }
    combo->addItem(i18n("Custom"), -1);
}

void ImageScalingWidget::initWriteImageFormat()
{
    /* Too many format :)
    QList<QByteArray> listWriteFormat = QImageWriter::supportedImageFormats();
    Q_FOREACH(const QByteArray& format, listWriteFormat) {
        d->ui->WriteToImageFormat->addItem(QString::fromLatin1(format));
    }
    */
    //known by several mailer.
    d->ui->WriteToImageFormat->addItem(QStringLiteral("JPG"));
    d->ui->WriteToImageFormat->addItem(QStringLiteral("PNG"));
}

void ImageScalingWidget::updateSettings()
{
    d->ui->enabledAutoResize->setChecked(MessageComposer::MessageComposerSettings::self()->autoResizeImageEnabled());
    d->ui->KeepImageRatio->setChecked(MessageComposer::MessageComposerSettings::self()->keepImageRatio());
    d->ui->AskBeforeResizing->setChecked(MessageComposer::MessageComposerSettings::self()->askBeforeResizing());
    d->ui->EnlargeImageToMinimum->setChecked(MessageComposer::MessageComposerSettings::self()->enlargeImageToMinimum());
    d->ui->ReduceImageToMaximum->setChecked(MessageComposer::MessageComposerSettings::self()->reduceImageToMaximum());
    d->ui->skipImageSizeLower->setChecked(MessageComposer::MessageComposerSettings::self()->skipImageLowerSizeEnabled());
    d->ui->imageSize->setValue(MessageComposer::MessageComposerSettings::self()->skipImageLowerSize());

    d->ui->customMaximumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumWidth());
    d->ui->customMaximumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMaximumHeight());
    d->ui->customMinimumWidth->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumWidth());
    d->ui->customMinimumHeight->setValue(MessageComposer::MessageComposerSettings::self()->customMinimumHeight());

    int index = qMax(0, d->ui->CBMaximumWidth->findData(MessageComposer::MessageComposerSettings::self()->maximumWidth()));
    d->ui->CBMaximumWidth->setCurrentIndex(index);
    d->ui->customMaximumWidth->setEnabled(d->ui->CBMaximumWidth->itemData(index) == -1);

    index = qMax(0, d->ui->CBMaximumHeight->findData(MessageComposer::MessageComposerSettings::self()->maximumHeight()));
    d->ui->CBMaximumHeight->setCurrentIndex(index);
    d->ui->customMaximumHeight->setEnabled(d->ui->CBMaximumHeight->itemData(index) == -1);

    index = qMax(0, d->ui->CBMinimumWidth->findData(MessageComposer::MessageComposerSettings::self()->minimumWidth()));
    d->ui->CBMinimumWidth->setCurrentIndex(index);
    d->ui->customMinimumWidth->setEnabled(d->ui->CBMinimumWidth->itemData(index) == -1);

    index = qMax(0, d->ui->CBMinimumHeight->findData(MessageComposer::MessageComposerSettings::self()->minimumHeight()));
    d->ui->CBMinimumHeight->setCurrentIndex(index);
    d->ui->customMinimumHeight->setEnabled(d->ui->CBMinimumHeight->itemData(index) == -1);

    index = d->ui->WriteToImageFormat->findData(MessageComposer::MessageComposerSettings::self()->writeFormat());
    if (index == -1) {
        d->ui->WriteToImageFormat->setCurrentIndex(0);
    } else {
        d->ui->WriteToImageFormat->setCurrentIndex(index);
    }
    d->ui->pattern->setText(MessageComposer::MessageComposerSettings::self()->filterSourcePattern());

    d->ui->renameResizedImage->setChecked(MessageComposer::MessageComposerSettings::self()->renameResizedImages());

    d->ui->renameResizedImagePattern->setText(MessageComposer::MessageComposerSettings::self()->renameResizedImagesPattern());
    d->ui->renameResizedImagePattern->setEnabled(d->ui->renameResizedImage->isChecked());

    d->ui->doNotResizePattern->setText(MessageComposer::MessageComposerSettings::self()->doNotResizeEmailsPattern());
    d->ui->resizeEmailsPattern->setText(MessageComposer::MessageComposerSettings::self()->resizeEmailsPattern());

    d->ui->resizeImageWithFormats->setChecked(MessageComposer::MessageComposerSettings::self()->resizeImagesWithFormats());
    d->ui->resizeImageWithFormatsType->setFormat(MessageComposer::MessageComposerSettings::self()->resizeImagesWithFormatsType());
    d->ui->resizeImageWithFormatsType->setEnabled(d->ui->resizeImageWithFormats->isChecked());

    updateFilterSourceTypeSettings();
    updateEmailsFilterTypeSettings();
}

void ImageScalingWidget::loadConfig()
{
    updateSettings();
    d->mWasChanged = false;
}

void ImageScalingWidget::updateFilterSourceTypeSettings()
{
    switch (MessageComposer::MessageComposerSettings::self()->filterSourceType()) {
    case MessageComposer::MessageComposerSettings::EnumFilterSourceType::NoFilter:
        d->ui->notFilterFilename->setChecked(true);
        d->ui->pattern->setEnabled(false);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterSourceType::IncludeFilesWithPattern:
        d->ui->includeFilesWithPattern->setChecked(true);
        d->ui->pattern->setEnabled(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterSourceType::ExcludeFilesWithPattern:
        d->ui->excludeFilesWithPattern->setChecked(true);
        d->ui->pattern->setEnabled(true);
        break;
    }
}

void ImageScalingWidget::updateEmailsFilterTypeSettings()
{
    d->ui->doNotResizePattern->setEnabled(false);
    d->ui->resizeEmailsPattern->setEnabled(false);

    switch (MessageComposer::MessageComposerSettings::self()->filterRecipientType()) {
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::NoFilter:
        d->ui->doNotFilterRecipients->setChecked(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeEachEmailsContainsPattern:
        d->ui->resizeEachEmails->setChecked(true);
        d->ui->resizeEmailsPattern->setEnabled(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::ResizeOneEmailContainsPattern:
        d->ui->resizeOneEmails->setChecked(true);
        d->ui->resizeEmailsPattern->setEnabled(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeEachEmailsContainsPattern:
        d->ui->doNotResizeEachEmails->setChecked(true);
        d->ui->doNotResizePattern->setEnabled(true);
        break;
    case MessageComposer::MessageComposerSettings::EnumFilterRecipientType::DontResizeOneEmailContainsPattern:
        d->ui->doNotResizeOneEmails->setChecked(true);
        d->ui->doNotResizePattern->setEnabled(false);
        break;
    }
}

void ImageScalingWidget::writeConfig()
{
    if (d->ui->EnlargeImageToMinimum->isChecked() && d->ui->ReduceImageToMaximum->isChecked()) {
        if ((d->ui->customMinimumWidth->value() >= d->ui->customMaximumWidth->value()) ||
                (d->ui->customMinimumHeight->value() >= d->ui->customMaximumHeight->value())) {
            KMessageBox::error(this, i18n("Please verify minimum and maximum values."), i18n("Error in minimum Maximum value"));
            return;
        }
    }
    MessageComposer::MessageComposerSettings::self()->setAutoResizeImageEnabled(d->ui->enabledAutoResize->isChecked());
    MessageComposer::MessageComposerSettings::self()->setKeepImageRatio(d->ui->KeepImageRatio->isChecked());
    MessageComposer::MessageComposerSettings::self()->setAskBeforeResizing(d->ui->AskBeforeResizing->isChecked());
    MessageComposer::MessageComposerSettings::self()->setEnlargeImageToMinimum(d->ui->EnlargeImageToMinimum->isChecked());
    MessageComposer::MessageComposerSettings::self()->setReduceImageToMaximum(d->ui->ReduceImageToMaximum->isChecked());

    MessageComposer::MessageComposerSettings::self()->setCustomMaximumWidth(d->ui->customMaximumWidth->value());
    MessageComposer::MessageComposerSettings::self()->setCustomMaximumHeight(d->ui->customMaximumHeight->value());
    MessageComposer::MessageComposerSettings::self()->setCustomMinimumWidth(d->ui->customMinimumWidth->value());
    MessageComposer::MessageComposerSettings::self()->setCustomMinimumHeight(d->ui->customMinimumHeight->value());

    MessageComposer::MessageComposerSettings::self()->setMaximumWidth(d->ui->CBMaximumWidth->itemData(d->ui->CBMaximumWidth->currentIndex()).toInt());
    MessageComposer::MessageComposerSettings::self()->setMaximumHeight(d->ui->CBMaximumHeight->itemData(d->ui->CBMaximumHeight->currentIndex()).toInt());
    MessageComposer::MessageComposerSettings::self()->setMinimumWidth(d->ui->CBMinimumWidth->itemData(d->ui->CBMinimumWidth->currentIndex()).toInt());
    MessageComposer::MessageComposerSettings::self()->setMinimumHeight(d->ui->CBMinimumHeight->itemData(d->ui->CBMinimumHeight->currentIndex()).toInt());

    MessageComposer::MessageComposerSettings::self()->setWriteFormat(d->ui->WriteToImageFormat->currentText());
    MessageComposer::MessageComposerSettings::self()->setSkipImageLowerSizeEnabled(d->ui->skipImageSizeLower->isChecked());
    MessageComposer::MessageComposerSettings::self()->setSkipImageLowerSize(d->ui->imageSize->value());

    MessageComposer::MessageComposerSettings::self()->setFilterSourcePattern(d->ui->pattern->text());

    MessageComposer::MessageComposerSettings::self()->setFilterSourceType(d->mSourceFilenameFilterGroup->checkedId());

    MessageComposer::MessageComposerSettings::self()->setRenameResizedImages(d->ui->renameResizedImage->isChecked());

    MessageComposer::MessageComposerSettings::self()->setRenameResizedImagesPattern(d->ui->renameResizedImagePattern->text());

    MessageComposer::MessageComposerSettings::self()->setDoNotResizeEmailsPattern(d->ui->doNotResizePattern->text());
    MessageComposer::MessageComposerSettings::self()->setResizeEmailsPattern(d->ui->resizeEmailsPattern->text());
    MessageComposer::MessageComposerSettings::self()->setFilterRecipientType(d->mRecipientFilterGroup->checkedId());

    MessageComposer::MessageComposerSettings::self()->setResizeImagesWithFormats(d->ui->resizeImageWithFormats->isChecked());
    MessageComposer::MessageComposerSettings::self()->setResizeImagesWithFormatsType(d->ui->resizeImageWithFormatsType->format());
    d->mWasChanged = false;
}

void ImageScalingWidget::resetToDefault()
{
    const bool bUseDefaults = MessageComposer::MessageComposerSettings::self()->useDefaults(true);
    updateSettings();
    MessageComposer::MessageComposerSettings::self()->useDefaults(bUseDefaults);
}

