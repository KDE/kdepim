/*
 * Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
 * Copyright (c) 2010 Tobias Koenig <tokoe@kdab.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MAILCOMMON_ACLENTRYDIALOG_P_H
#define MAILCOMMON_ACLENTRYDIALOG_P_H

#include <KImap/kimap/Acl>

#include <QDialog>

namespace PimCommon {

/**
 * @short A dialog for editing an IMAP ACL entry.
 */
class AclEntryDialog : public QDialog
{
  Q_OBJECT

  public:
    /**
     * Creates a new ACL entry dialog.
     *
     * @param parent The parent widget.
     */
    explicit AclEntryDialog( QWidget *parent = 0 );

    /**
     * Destroys the ACL entry dialog.
     */
    ~AclEntryDialog();

    /**
     * Sets the user @p id of the ACL entry.
     */
    void setUserId( const QString &id );

    /**
     * Returns the user id of the ACL entry.
     */
    QString userId() const;

    /**
     * Sets the permissions of the ACL entry.
     */
    void setPermissions( KIMAP::Acl::Rights permissions );

    /**
     * Returns the permissions of the ACL entry.
     */
    KIMAP::Acl::Rights permissions() const;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void slotSelectAddresses() )
    Q_PRIVATE_SLOT( d, void slotChanged() )
    //@endcond
};

}

#endif
