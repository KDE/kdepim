/*
  This file is part of libkdepim.

  Copyright (c) 2004 Lutz Rogowski <rogowski@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KPIM_KINCIDENCECHOOSER_H
#define KPIM_KINCIDENCECHOOSER_H

#include <kdialogbase.h>

namespace KCal {
  class Incidence;
}

class QButtonGroup;
class QGridLayout;
class QRadioButton;

namespace KPIM {

/**
 * Dialog to deal with conflicts encountered when modifying the contents of shared folders.
 */
class KDE_EXPORT KIncidenceChooser : public KDialog
{
  Q_OBJECT
  public:
    enum TakeMode {
      Newer=0,  /**< take the newer of the two incidences in conflict */
      Remote,   /**< take the server copy of the incidence */
      Local,    /**< take the local copy of the incidence */
      Both      /**< take both incidences */
    };

    enum ConflictAskPolicy {
      Always=0,  /**< always ask */
      Sync,      /**< ask on first conflict per sync only */
      Session,   /**< ask on first conflict per session only */
      Never      /**< never ask */
    };

    /** Initialize dialog and pages */
    explicit KIncidenceChooser( const QString &folder,
                                ConflictAskPolicy askPolicy=Always,
                                bool FolderOnly=true,
                                QWidget *parent=0, char *name=0 );
    ~KIncidenceChooser();
    void setIncidences( KCal::Incidence *, KCal::Incidence  * );

    KCal::Incidence *takeIncidence();
    TakeMode takeMode();

    void setConflictAskPolicy( ConflictAskPolicy policy );
    ConflictAskPolicy conflictAskPolicy();

    void setFolderOnly( bool folderOnly );
    bool folderOnly();

  public slots:
    void useGlobalMode();

  protected slots:
    void showLocalIncidence();
    void showRemoteIncidence();
    void takeNewerIncidence();
    void takeRemoteIncidence();
    void takeLocalIncidence();
    void takeBothIncidence();
    void setLabels();
    void detailsDialogClosed();
    void slotFolderAll();
    void slotFolderOnly();

  protected:
    void keyPressEvent( QKeyEvent *e );
    void closeEvent( QCloseEvent *e );

  private:
    QString summaryStr( KCal::Incidence *incidence ) const;
    QString modifiedStr( KCal::Incidence *incidence ) const;
    QString mFolder;
    KCal::Incidence *mLocInc, *mRemInc, *mSelIncidence;
    QButtonGroup *mBg;
    QPushButton *mLocShowDetails, *mRemShowDetails;
    QLabel *mLocEntryVal, *mLocModVal;
    QLabel *mRemEntryVal, *mRemModVal;
    KDialogBase *mTbL, *mTbN;
    QRadioButton *mFolderAllBut, *mFolderOnlyBut;
    ConflictAskPolicy mAskPolicy;
    bool mFolderOnly;
    TakeMode mTakeMode;
};

}

#endif
