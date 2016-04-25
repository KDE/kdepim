/*
  Copyright (c) 2012-2016 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "composerimagedialog.h"
//#include "extendattributes/extendattributesbutton.h"

#include <kpimtextedit/insertimagewidget.h>

#include <KLocalizedString>
#include <QLineEdit>
#include <KSeparator>
#include <QUrl>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

namespace ComposerEditorWebEngine
{

class ComposerImageDialogPrivate
{
public:
    ComposerImageDialogPrivate(ComposerImageDialog *qq)
        : q(qq)
    {
    }

    void _k_slotOkClicked();
    void _k_slotApplyClicked();
    void _k_slotWebElementChanged();
    void _k_slotEnableButtonOk(bool b);
    void initialize();

    QString html() const;

    void updateImageHtml();
    void updateSettings();

    //QWebElement webElement;

    KPIMTextEdit::InsertImageWidget *imageWidget;
    QLineEdit *title;
    QLineEdit *alternateTitle;
    QPushButton *okButton;
    ComposerImageDialog *q;
};

void ComposerImageDialogPrivate::_k_slotEnableButtonOk(bool b)
{
    okButton->setEnabled(b);
}

void ComposerImageDialogPrivate::_k_slotWebElementChanged()
{
    updateSettings();
}

void ComposerImageDialogPrivate::_k_slotOkClicked()
{
#if 0
    if (!webElement.isNull()) {
        updateImageHtml();
    }
#endif
    q->accept();
}

void ComposerImageDialogPrivate::_k_slotApplyClicked()
{
#if 0
    if (!webElement.isNull()) {
        updateImageHtml();
    }
#endif
}

void ComposerImageDialogPrivate::updateImageHtml()
{
#if 0
    int imageWidth = -1;
    int imageHeight = -1;
    if (!imageWidget->keepOriginalSize()) {
        imageWidth = imageWidget->imageWidth();
        imageHeight = imageWidget->imageHeight();
    }
    if (imageWidth == -1) {
        webElement.removeAttribute(QStringLiteral("width"));
    } else {
        webElement.setAttribute(QStringLiteral("width"), QString::number(imageWidth));
    }

    if (imageHeight == -1) {
        webElement.removeAttribute(QStringLiteral("height"));
    } else {
        webElement.setAttribute(QStringLiteral("height"), QString::number(imageHeight));
    }

    QString str(title->text());
    if (str.isEmpty()) {
        webElement.removeAttribute(QStringLiteral("title"));
    } else {
        webElement.setAttribute(QStringLiteral("title"), str);
    }

    str = alternateTitle->text();
    if (str.isEmpty()) {
        webElement.removeAttribute(QStringLiteral("alt"));
    } else {
        webElement.setAttribute(QStringLiteral("alt"), str);
    }

    webElement.setAttribute(QStringLiteral("src"), imageWidget->imageUrl().url());
#endif
}

void ComposerImageDialogPrivate::initialize()
{
    //q->setWindowTitle(webElement.isNull() ? i18n("Insert Image") : i18n("Edit Image"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(/*webElement.isNull() ? */QDialogButtonBox::Ok /*: QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply*/);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    q->setLayout(mainLayout);
    q->connect(buttonBox, SIGNAL(accepted()), q, SLOT(_k_slotOkClicked()));
    q->connect(buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setText(i18n("Insert"));

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    lay->setSpacing(0);
    w->setLayout(lay);

    imageWidget = new KPIMTextEdit::InsertImageWidget(q);
    lay->addWidget(imageWidget);

    KSeparator *sep = new KSeparator;
    lay->addWidget(sep);

    //ToolTip
    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *lab = new QLabel(i18n("Tooltip:"));
    hbox->addWidget(lab);
    title = new QLineEdit;
    title->setClearButtonEnabled(true);
    hbox->addWidget(title);
    lay->addLayout(hbox);

    //Alternate text
    hbox = new QHBoxLayout;
    lab = new QLabel(i18n("Alternate text:"));
    hbox->addWidget(lab);
    alternateTitle = new QLineEdit;
    alternateTitle->setClearButtonEnabled(true);
    hbox->addWidget(alternateTitle);
    lay->addLayout(hbox);

#if 0
    if (!webElement.isNull()) {
        ExtendAttributesButton *button = new ExtendAttributesButton(webElement, ExtendAttributesDialog::Image, q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        lay->addWidget(button);
        q->connect(q, SIGNAL(clicked()), q, SLOT(_k_slotApplyClicked()));
    }
#endif
    sep = new KSeparator;
    lay->addWidget(sep);

    q->connect(imageWidget, SIGNAL(enableButtonOk(bool)), q, SLOT(_k_slotEnableButtonOk(bool)));

    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
    okButton->setEnabled(false);

    updateSettings();
}

void ComposerImageDialogPrivate::updateSettings()
{
#if 0
    imageWidget->setImageUrl(QUrl(webElement.attribute(QStringLiteral("src"))));
    if (webElement.hasAttribute(QStringLiteral("height")) && webElement.hasAttribute(QStringLiteral("width"))) {
        imageWidget->setImageWidth(webElement.attribute(QStringLiteral("width")).toInt());
        imageWidget->setImageHeight(webElement.attribute(QStringLiteral("height")).toInt());
    }
    alternateTitle->setText(webElement.attribute(QStringLiteral("alt")));
    title->setText(webElement.attribute(QStringLiteral("title")));
#endif
}

QString ComposerImageDialogPrivate::html() const
{
    const QUrl url = imageWidget->imageUrl();
    int imageWidth = -1;
    int imageHeight = -1;
    if (!imageWidget->keepOriginalSize()) {
        imageWidth = imageWidget->imageWidth();
        imageHeight = imageWidget->imageHeight();
    }
    QString imageHtml = QStringLiteral("<img");
    if (imageWidth > 0) {
        imageHtml += QStringLiteral(" width=%1").arg(imageWidth);
    }
    if (imageHeight > 0) {
        imageHtml += QStringLiteral(" height=%1").arg(imageHeight);
    }
    if (!url.isEmpty()) {
        imageHtml += QStringLiteral(" src='file://%1'").arg(url.path());
    }
    QString str = title->text();
    if (!str.isEmpty()) {
        imageHtml += QStringLiteral(" title='%1'").arg(str);
    }
    str = alternateTitle->text();
    if (!str.isEmpty()) {
        imageHtml += QStringLiteral(" alt='%1'").arg(str);
    }
    imageHtml += QLatin1String(" />");
    return imageHtml;
}

ComposerImageDialog::ComposerImageDialog(QWidget *parent)
    : QDialog(parent), d(new ComposerImageDialogPrivate(this))
{
    d->initialize();
}

#if 0
ComposerImageDialog::ComposerImageDialog(const QWebElement &element, QWidget *parent)
    : QDialog(parent), d(new ComposerImageDialogPrivate(this))
{
    d->webElement = element;
    d->initialize();
}
#endif

ComposerImageDialog::~ComposerImageDialog()
{
    delete d;
}

QString ComposerImageDialog::html() const
{
    return d->html();
}

}

#include "moc_composerimagedialog.cpp"
