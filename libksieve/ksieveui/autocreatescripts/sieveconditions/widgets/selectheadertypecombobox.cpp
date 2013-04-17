/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include <KLocale>
#include <KLineEdit>

using namespace KSieveUi;

static const char selectMultipleHeaders[] = I18N_NOOP( "Select multiple headers..." );


SelectHeadersDialog::SelectHeadersDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Headers" ) );
    setButtons( Ok|Cancel );
    setButtonFocus( Ok );
    mListWidget = new SelectHeadersWidget;
    setMainWidget(mListWidget);
}

SelectHeadersDialog::~SelectHeadersDialog()
{
}

void SelectHeadersDialog::setListHeaders(const QMap<QString, QString> &lst)
{
    mListWidget->setListHeaders(lst);
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

void SelectHeadersWidget::setListHeaders(const QMap<QString, QString> &lst)
{

    QMapIterator<QString, QString> i(lst);
    while (i.hasNext()) {
        i.next();
        QListWidgetItem *item = new QListWidgetItem(i.value(), this);
        item->setData(HeaderId, i.key());
        item->setCheckState(Qt::Unchecked);
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

SelectHeaderTypeComboBox::SelectHeaderTypeComboBox(QWidget *parent)
    : KComboBox(parent)
{
    setEditable(true);
    //TODO add completion
    initialize();
    connect(this, SIGNAL(activated(QString)), SLOT(slotSelectItem(QString)));
}

SelectHeaderTypeComboBox::~SelectHeaderTypeComboBox()
{
}

void SelectHeaderTypeComboBox::slotSelectItem(const QString &str)
{
    if (str == i18n(selectMultipleHeaders)) {
        QPointer<SelectHeadersDialog> dlg = new SelectHeadersDialog(this);
        dlg->setListHeaders(mHeaderMap);
        if (dlg->exec()) {
            lineEdit()->setText(dlg->headers());
        } else {
            lineEdit()->clear();
        }
        delete dlg;
    }
}

void SelectHeaderTypeComboBox::headerMap()
{
    mHeaderMap.insert(QLatin1String("from"), i18n("From"));
    mHeaderMap.insert(QLatin1String("to"), i18n("To"));
    mHeaderMap.insert(QLatin1String("cc"), i18n("Cc"));
    mHeaderMap.insert(QLatin1String("bcc"), i18n("Bcc"));
    mHeaderMap.insert(QLatin1String("sender"), i18n("Sender"));
    mHeaderMap.insert(QLatin1String("sender-from"), i18n("Sender-From"));
    mHeaderMap.insert(QLatin1String("sender-to"), i18n("Sender-To"));
}

void SelectHeaderTypeComboBox::initialize()
{
    headerMap();
    QMapIterator<QString, QString> i(mHeaderMap);
    while (i.hasNext()) {
        i.next();
        addItem(i.value(), i.key());
    }
    addItem(i18n(selectMultipleHeaders));
}

QString SelectHeaderTypeComboBox::code() const
{
    QString str = itemData(currentIndex()).toString();
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

#include "selectheadertypecombobox.moc"
