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

#ifndef SELECTHEADERTYPECOMBOBOX_H
#define SELECTHEADERTYPECOMBOBOX_H

#include <KComboBox>
#include <KDialog>

#include <QListWidget>

class QLineEdit;
class QPushButton;
namespace KSieveUi {

class SelectHeadersWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit SelectHeadersWidget(QWidget *parent = 0);
    ~SelectHeadersWidget();

    QString headers() const;
    void setListHeaders(const QMap<QString, QString> &lst, const QStringList &selectedHeaders);
    void addNewHeader(const QString &header);
private:
    enum HeaderEnum {
        HeaderId = Qt::UserRole + 1
    };

    void init();
};

class SelectHeadersDialog : public KDialog
{
    Q_OBJECT
public:
    explicit SelectHeadersDialog(QWidget *parent = 0);
    ~SelectHeadersDialog();

    QString headers() const;
    void setListHeaders(const QMap<QString, QString> &lst, const QStringList &selectedHeaders);

private Q_SLOTS:
    void slotNewHeaderTextChanged(const QString &text);
    void slotAddNewHeader();

private:
    void readConfig();
    void writeConfig();
    SelectHeadersWidget *mListWidget;
    QLineEdit *mNewHeader;
    QPushButton *mAddNewHeader;
};


class SelectHeaderTypeComboBox : public KComboBox
{
    Q_OBJECT
public:
    explicit SelectHeaderTypeComboBox(bool onlyEnvelopType = false, QWidget *parent = 0);
    ~SelectHeaderTypeComboBox();

    QString code() const;
    void setCode(const QString &code);

Q_SIGNALS:
    void valueChanged();

private Q_SLOTS:
    void slotSelectItem(const QString &str);

private:
    void initialize(bool onlyEnvelopType);
    void headerMap(bool onlyEnvelopType);
    QMap<QString, QString> mHeaderMap;
    QString mCode;
};

}

#endif // SELECTHEADERTYPECOMBOBOX_H
