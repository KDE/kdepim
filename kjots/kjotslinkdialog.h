//
//  kjots
//
//  Copyright (C) 2008 Stephen Kelly <steveire@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef KJOTSLINKDIALOG_H
#define KJOTSLINKDIALOG_H

#include <QDialog>

class QLabel;
class QAbstractItemModel;
class QString;
class QRadioButton;
class QTreeView;

class KComboBox;
class QLineEdit;

class KJotsLinkDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KJotsLinkDialog(QAbstractItemModel *kjotsModel, QWidget *parent = 0);

    /**
     * Returns the link text shown in the dialog
     * @param linkText The initial text
     */
    void setLinkText(const QString &linkText);

    /**
     * Sets the target link url shown in the dialog
     * @param linkUrl The initial link target url
     */
    void setLinkUrl(const QString &linkUrl);

    /**
     * Returns the link text entered by the user.
     * @return The link text
     */
    QString linkText() const;

    /**
     * Returns the target link url entered by the user.
     * @return The link url
     */
    QString linkUrl() const;

public slots:
    void trySetEntry(const QString &text);

private:
    QLabel *textLabel;
    QLineEdit *textLineEdit;
    QLabel *linkUrlLabel;
    QLineEdit *linkUrlLineEdit;
    KComboBox *hrefCombo;
    QRadioButton *linkUrlLineEditRadioButton;
    QRadioButton *hrefComboRadioButton;
    QTreeView *tree;
    QAbstractItemModel *m_kjotsModel;
    QAbstractItemModel *m_descendantsProxyModel;
};

#endif
