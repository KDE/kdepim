/* Copyright 2010 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MESSAGECORE_ANNOTATIONDIALOG_H
#define MESSAGECORE_ANNOTATIONDIALOG_H

#include "messagecore_export.h"

#include <KDialog>

namespace MessageCore {

/**
 * @short A dialog for editing annotations of an email.
 *
 * @author Thomas McGuire <mcguire@kde.org>
 */
class MESSAGECORE_EXPORT AnnotationEditDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * Creates a new annotation edit dialog.
     *
     * @param uri The Nepomuk resource uri of the email.
     * @param parent The parent widget.
     */
    AnnotationEditDialog( const QUrl &uri, QWidget *parent = 0 );

    /**
     * Destroys the annotation edit dialog.
     */
    ~AnnotationEditDialog();

  protected:
    /// Reimplemented to handle button clicks
    virtual void slotButtonClicked( int button );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
};

}

#endif
