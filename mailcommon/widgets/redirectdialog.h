/*  -*- mode: C++ -*-

  Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>
  Copyright (c) 2014 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
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
#ifndef MAILCOMMON_REDIRECTDIALOG_H
#define MAILCOMMON_REDIRECTDIALOG_H

#include "mailcommon_export.h"

#include <QDialog>
#include <KConfigGroup>
class QFormLayout;

namespace MessageComposer
{
class ComposerLineEdit;
}

namespace MailCommon
{

class RedirectWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RedirectWidget(QWidget *parent = 0);
    ~RedirectWidget();

    void setFocus();
    QString resend();

Q_SIGNALS:
    void addressChanged(const QString &);

private slots:
    void slotAddressSelection();

private:
    MessageComposer::ComposerLineEdit *mEdit;
    QString mResendStr;
};

/**
 * @short A dialog to request information about message redirection from the user.
 *
 * The dialog is used to collect redirect addresses when
 * manually redirecting messages. Only Redirect-To is
 * supported so far.
 *
 * @author Andreas Gungl <a.gungl@gmx.de>
 */
class MAILCOMMON_EXPORT RedirectDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Describes the send mode.
     */
    enum SendMode {
        SendNow,
        SendLater
    };

    /**
     * Creates a new redirect dialog.
     *
     * @param mode The preferred send mode.
     * @param parent The parent widget.
     */
    explicit RedirectDialog(SendMode mode = SendNow, QWidget *parent = 0);

    /**
     * Destroys the redirect dialog.
     */
    ~RedirectDialog();

    /**
     * Returns the addresses for the redirection.
     */
    QString to() const;

    /**
     * Returns the send mode.
     */
    SendMode sendMode() const;

    int transportId() const;

    int identity() const;

    QString cc() const;
    QString bcc() const;
protected:
    virtual void accept();

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void slotUser1())
    Q_PRIVATE_SLOT(d, void slotUser2())
    Q_PRIVATE_SLOT(d, void slotAddressChanged(const QString &))
    //@endcond
};

}

#endif
