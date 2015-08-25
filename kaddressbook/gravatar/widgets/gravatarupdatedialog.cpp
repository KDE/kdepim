/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "gravatarupdatedialog.h"
#include "gravatarupdatewidget.h"
#include <QVBoxLayout>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace KABGravatar;

GravatarUpdateDialog::GravatarUpdateDialog(QWidget *parent)
    : QDialog(parent),
      mSaveUrl(false)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    setWindowTitle(i18n("Check and update gravatar"));
    mGravatarUpdateWidget = new GravatarUpdateWidget(this);
    connect(mGravatarUpdateWidget, &GravatarUpdateWidget::activateDialogButton, this, &GravatarUpdateDialog::slotActivateButton);
    mGravatarUpdateWidget->setObjectName(QStringLiteral("gravatarupdatewidget"));
    mainLayout->addWidget(mGravatarUpdateWidget);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    buttonBox->setObjectName(QStringLiteral("buttonbox"));
    mSaveImageButton = new QPushButton(i18n("Save Image"), this);
    mSaveImageButton->setEnabled(false);
    buttonBox->addButton(mSaveImageButton, QDialogButtonBox::ActionRole);
    connect(mSaveImageButton, &QPushButton::clicked, this, &GravatarUpdateDialog::slotSaveImage);

    mSaveUrlButton = new QPushButton(i18n("Save Image Url"), this);
    buttonBox->addButton(mSaveUrlButton, QDialogButtonBox::ActionRole);
    mSaveUrlButton->setEnabled(false);
    connect(mSaveUrlButton, &QPushButton::clicked, this, &GravatarUpdateDialog::slotSaveUrl);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

GravatarUpdateDialog::~GravatarUpdateDialog()
{
    writeConfig();
}

void GravatarUpdateDialog::slotActivateButton(bool state)
{
    mSaveUrlButton->setEnabled(state);
    mSaveImageButton->setEnabled(state);
}

bool GravatarUpdateDialog::saveUrl() const
{
    return mSaveUrl;
}

void GravatarUpdateDialog::slotSaveUrl()
{
    mSaveUrl = true;
    accept();
}

void GravatarUpdateDialog::slotSaveImage()
{
    mSaveUrl = false;
    accept();
}

void GravatarUpdateDialog::setEmail(const QString &email)
{
    mGravatarUpdateWidget->setEmail(email);
}

QPixmap GravatarUpdateDialog::pixmap() const
{
    return mGravatarUpdateWidget->pixmap();
}

void GravatarUpdateDialog::setOriginalUrl(const QString &url)
{
    mGravatarUpdateWidget->setOriginalUrl(url);
}

QUrl GravatarUpdateDialog::resolvedUrl() const
{
    return mGravatarUpdateWidget->resolvedUrl();
}

void GravatarUpdateDialog::setOriginalPixmap(const QPixmap &pix)
{
    mGravatarUpdateWidget->setOriginalPixmap(pix);
}

void GravatarUpdateDialog::readConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "GravatarUpdateDialog");
    const QSize size = grp.readEntry("Size", QSize(300, 200));
    if (size.isValid()) {
        resize(size);
    }
}

void GravatarUpdateDialog::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "GravatarUpdateDialog");
    grp.writeEntry("Size", size());
    grp.sync();
}

void GravatarUpdateDialog::slotAccepted()
{
    accept();
}
