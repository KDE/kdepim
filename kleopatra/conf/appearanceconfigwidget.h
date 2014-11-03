/*
    appearanceconfigwidget.h

    This file is part of kleopatra, the KDE key manager
    Copyright (c) 2002,2004,2008 Klarälvdalens Datakonsult AB
    Copyright (c) 2002,2003 Marc Mutz <mutz@kde.org>

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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

#ifndef __KLEOPATRA_CONFIG_APPEARANCECONFIGWIDGET_H__
#define __KLEOPATRA_CONFIG_APPEARANCECONFIGWIDGET_H__

#include <QWidget>

#include <utils/pimpl_ptr.h>

namespace Kleo
{
namespace Config
{

class AppearanceConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppearanceConfigWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~AppearanceConfigWidget();

public Q_SLOTS:
    void load();
    void save();
    void defaults();

Q_SIGNALS:
    void changed();

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotIconClicked())
#ifndef QT_NO_COLORDIALOG
    Q_PRIVATE_SLOT(d, void slotForegroundClicked())
    Q_PRIVATE_SLOT(d, void slotBackgroundClicked())
#endif
#ifndef QT_NO_FONTDIALOG
    Q_PRIVATE_SLOT(d, void slotFontClicked())
#endif
    Q_PRIVATE_SLOT(d, void slotSelectionChanged())
    Q_PRIVATE_SLOT(d, void slotDefaultClicked())
    Q_PRIVATE_SLOT(d, void slotItalicToggled(bool))
    Q_PRIVATE_SLOT(d, void slotBoldToggled(bool))
    Q_PRIVATE_SLOT(d, void slotStrikeOutToggled(bool))
    Q_PRIVATE_SLOT(d, void slotTooltipValidityChanged(bool))
    Q_PRIVATE_SLOT(d, void slotTooltipDetailsChanged(bool))
    Q_PRIVATE_SLOT(d, void slotTooltipOwnerChanged(bool))
};

}
}

#endif // __KLEOPATRA_CONFIG_APPEARANCECONFIGWIDGET_H__
