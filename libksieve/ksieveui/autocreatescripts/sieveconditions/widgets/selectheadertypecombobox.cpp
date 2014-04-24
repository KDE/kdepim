/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "selectheadertypecombobox.h"
#include "autocreatescripts/autocreatescriptutil_p.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <KPushButton>
#include <KIcon>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

using namespace KSieveUi;

static const char selectMultipleHeaders[] = I18N_NOOP( "Select multiple headers..." );


SelectHeadersDialog::SelectHeadersDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Headers" ) );
    setButtons( Ok|Cancel );
    setButtonFocus( Ok );
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);
    mListWidget = new SelectHeadersWidget;
    lay->addWidget(mListWidget);

    QLabel *lab = new QLabel(i18n("Add new header:"));
    lay->addWidget(lab);

    QHBoxLayout *hbox = new QHBoxLayout;

    mNewHeader = new KLineEdit;
    mNewHeader->setClearButtonShown(true);
    hbox->addWidget(mNewHeader);


    mAddNewHeader = new KPushButton;
    mAddNewHeader->setEnabled(false);
    mAddNewHeader->setIcon( KIcon( QLatin1String("list-add") ) );
    mAddNewHeader->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    connect(mAddNewHeader, SIGNAL(clicked(bool)), SLOT(slotAddNewHeader()));
    connect(mNewHeader, SIGNAL(textChanged(QString)), this, SLOT(slotNewHeaderTextChanged(QString)));
    hbox->addWidget(mAddNewHeader);

    lay->addLayout(hbox);

    setMainWidget(w);
    resize(400,300);
}

SelectHeadersDialog::~SelectHeadersDialog()
{
}

void SelectHeadersDialog::slotNewHeaderTextChanged(const QString &text)
{
    mAddNewHeader->setEnabled(!text.trimmed().isEmpty());
}

void SelectHeadersDialog::slotAddNewHeader()
{
    const QString headerText = mNewHeader->text().trimmed();
    if (!headerText.isEmpty())
        mListWidget->addNewHeader(headerText);
}

void SelectHeadersDialog::setListHeaders(const QMap<QString, QString> &lst, const QStringList &selectedHeaders)
{
    mListWidget->setListHeaders(lst, selectedHeaders);
}

QString SelectHeadersDialog::headers() const
{
    return mListWidget->headers();
}

SelectHeadersWidget::SelectHeadersWidget(QWidget *parent)
    : QListWidget(parent)
{
}

SelectHeadersWidget::~SelectHeadersWidget()
{
}

void SelectHeadersWidget::addNewHeader(const QString &header)
{
    QListWidgetItem *item = new QListWidgetItem(header, this);
    item->setData(HeaderId, header);
    item->setCheckState(Qt::Unchecked);
}

void SelectHeadersWidget::setListHeaders(const QMap<QString, QString> &lst, const QStringList &selectedHeaders)
{
    QMapIterator<QString, QString> i(lst);
    while (i.hasNext()) {
        i.next();
        QListWidgetItem *item = new QListWidgetItem(i.value(), this);
        item->setData(HeaderId, i.key());
        if (selectedHeaders.contains(i.key())) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
    Q_FOREACH (const QString &header, selectedHeaders) {
        if (!lst.contains(header)) {
            QListWidgetItem *item = new QListWidgetItem(header, this);
            item->setData(HeaderId, header);
            item->setCheckState(Qt::Checked);
        }
    }
}

QString SelectHeadersWidget::headers() const
{
    QString result;
    bool selected = false;
    const int numberOfItem = count();
    for (int i = 0; i < numberOfItem; ++i) {
        QListWidgetItem *it = item(i);
        if (it->checkState() == Qt::Checked) {
            if (selected) {
                result += QLatin1String(", ");
            }
            selected = true;
            result += QLatin1String("\"") + it->data(HeaderId).toString() + QLatin1String("\"");
        }
    }
    if (!result.isEmpty()) {
        result = QLatin1String("[ ") + result + QLatin1String(" ]");
    }
    return result;
}

SelectHeaderTypeComboBox::SelectHeaderTypeComboBox(bool onlyEnvelopType, QWidget *parent)
    : KComboBox(parent)
{
    setEditable(true);
    //TODO add completion
    initialize(onlyEnvelopType);
    connect(this, SIGNAL(activated(QString)), SLOT(slotSelectItem(QString)));
    connect(this, SIGNAL(textChanged(QString)), SIGNAL(valueChanged()));
    connect(this, SIGNAL(activated(int)), this, SIGNAL(valueChanged()));
}

SelectHeaderTypeComboBox::~SelectHeaderTypeComboBox()
{
}

void SelectHeaderTypeComboBox::slotSelectItem(const QString &str)
{
    if (str == i18n(selectMultipleHeaders)) {
        QPointer<SelectHeadersDialog> dlg = new SelectHeadersDialog(this);
        dlg->setListHeaders(mHeaderMap, AutoCreateScriptUtil::createListFromString(mCode));
        if (dlg->exec()) {
            mCode = dlg->headers();
            lineEdit()->setText(dlg->headers());
            Q_EMIT valueChanged();
        } else {
            lineEdit()->setText(mCode);
        }
        delete dlg;
    } else {
        mCode = str;
    }
}

void SelectHeaderTypeComboBox::headerMap(bool onlyEnvelopType)
{
    mHeaderMap.insert(QLatin1String("from"), i18n("From"));
    mHeaderMap.insert(QLatin1String("to"), i18n("To"));
    mHeaderMap.insert(QLatin1String("Reply-To"), i18n("Reply To"));
    mHeaderMap.insert(QLatin1String("cc"), i18n("Cc"));
    mHeaderMap.insert(QLatin1String("bcc"), i18n("Bcc"));
    if (!onlyEnvelopType) {
        mHeaderMap.insert(QLatin1String("Subject"), i18n("Subject"));
        mHeaderMap.insert(QLatin1String("sender"), i18n("Sender"));
        mHeaderMap.insert(QLatin1String("Date"), i18n("Date"));
        mHeaderMap.insert(QLatin1String("Message-ID"), i18n("Message Id"));
        mHeaderMap.insert(QLatin1String("Content-Type"), i18n("Content type"));
    }
}

void SelectHeaderTypeComboBox::initialize(bool onlyEnvelopType)
{
    headerMap(onlyEnvelopType);
    QMapIterator<QString, QString> i(mHeaderMap);
    while (i.hasNext()) {
        i.next();
        addItem(i.value(), i.key());
    }
    addItem(i18n(selectMultipleHeaders));
}

QString SelectHeaderTypeComboBox::code() const
{
    QString str = mCode.isEmpty() ? itemData(currentIndex()).toString() : mCode;
    if (str.isEmpty()) {
        str = currentText();
        if (str == i18n(selectMultipleHeaders)) {
            str = QString(); //return QString();
        }
    }
    if (!str.isEmpty() && !str.startsWith(QLatin1Char('['))) {
        str = QLatin1String("\"") + str + QLatin1String("\"");
    }
    return str;
}

void SelectHeaderTypeComboBox::setCode(const QString &code)
{
    lineEdit()->setText(code);
    mCode = code;
}

