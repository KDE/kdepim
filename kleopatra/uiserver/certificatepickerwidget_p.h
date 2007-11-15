/* -*- mode: c++; c-basic-offset:4 -*-
    certificatepickerwidget_p.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEOPATRA_CERTIFICATEPICKERWIDGET_P_H__
#define __KLEOPATRA_CERTIFICATEPICKERWIDGET_P_H__

#include <QString>
#include <QWidget>

#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;

namespace GpgME {
    class Key;
}

namespace Kleo {

class CertificatePickerWidget : public QWidget {
    Q_OBJECT

public:
    explicit CertificatePickerWidget( const QString& identifier,
                                    QWidget* parent = 0 );

    void setSuggestions( const std::vector<GpgME::Key>& keys );
    GpgME::Key selectedKey() const;
    bool rememberSelection() const;
    bool isComplete() const;

Q_SIGNALS:
    void completionStateChanged( const QString& id );

private:
    QVariant currentData() const;
    void maybeCompletionChanged();
    void addKey( const GpgME::Key& key );

private Q_SLOTS:
    void selectAnotherCertificate();
    void currentIndexChanged( int idx );

private:
    QString m_identifier;
    bool m_wasComplete;
    QComboBox* m_combo;
    QPushButton* m_selectButton;
    QCheckBox* m_rememberChoiceCO;
};

}

#endif // __KLEOPATRA_CERTIFICATEPICKERWIDGET_P_H__

