/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/wizardpage.h

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

#ifndef __KLEOPATRA_CRYPTO_GUI_WIZARDPAGE_H__
#define __KLEOPATRA_CRYPTO_GUI_WIZARDPAGE_H__

#include <QWidget>

#include <utils/pimpl_ptr.h>

class KGuiItem;

namespace Kleo
{
namespace Crypto
{
namespace Gui
{

class Wizard;

class WizardPage : public QWidget
{
    friend class ::Kleo::Crypto::Gui::Wizard;
    Q_OBJECT
public:
    explicit WizardPage(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~WizardPage();

    virtual bool isComplete() const = 0;

    bool isCommitPage() const;
    void setCommitPage(bool commitPage);

    bool autoAdvance() const;
    void setAutoAdvance(bool enabled);

    QString title() const;
    void setTitle(const QString &title);

    QString subTitle() const;
    void setSubTitle(const QString &subTitle);

    QString explanation() const;
    void setExplanation(const QString &explanation);

    KGuiItem customNextButton() const;
    void setCustomNextButton(const KGuiItem &item);

Q_SIGNALS:
    void completeChanged();
    void explanationChanged();
    void titleChanged();
    void subTitleChanged();
    void autoAdvanceChanged();
    void windowTitleChanged(const QString &title);

protected:

    virtual void onNext();

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

}
}
}

#endif // __KLEOPATRA_CRYPTO_GUI_WIZARDPAGE_H__

