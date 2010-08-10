/*
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Volker Krause <vkrause@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef ATTENDEEEDITOR_H
#define ATTENDEEEDITOR_H

#include <kcalcore/attendee.h>
#include <kcalcore/incidence.h>

#include <QWidget>

namespace KPIM {
  class AddresseeLineEdit;
}

namespace KABC {
  class Addressee;
}

namespace KCal {
  class Incidence;
}

class KComboBox;
class KHBox;

class QBoxLayout;
class QCheckBox;
class QLabel;
class QPushButton;
class Q3ListViewItem;

/**
  Common base class for attendee editor and free busy view.
*/
class AttendeeEditor : public QWidget
{
  Q_OBJECT
  public:
    AttendeeEditor( QWidget *parent );

    virtual void insertAttendee( const KCalCore::Attendee::Ptr &attendee, bool fetchFB = true ) = 0;

    virtual void readIncidence( const KCalCore::Incidence::Ptr &incidence );
    virtual void fillIncidence( KCalCore::Incidence::Ptr &incidence );

    /** return a clone of the incidence with attendees to be canceled */
    void cancelAttendeeIncidence( const KCalCore::Incidence::Ptr &incidence );

  public slots:
    void acceptForMe();
    void declineForMe();

  signals:
    void updateAttendeeSummary( int count );

  protected:
    void initOrganizerWidgets( QWidget *parent, QBoxLayout *layout );
    void initEditWidgets( QWidget *parent, QBoxLayout *layout );

    /** Reads values from a KABC::Addressee and inserts a new Attendee
     * item into the listview with those items. Used when adding attendees
     * from the addressbook and expanding distribution lists.
     * The optional Attendee parameter can be used to pass in default values
     * to be used by the new Attendee. */
    void insertAttendeeFromAddressee( const KABC::Addressee &a,
                                      const KCalCore::Attendee::Ptr &at = KCalCore::Attendee::Ptr() );

    void fillOrganizerCombo();
    virtual Q3ListViewItem *hasExampleAttendee() const = 0;
    bool isExampleAttendee( const KCalCore::Attendee::Ptr & ) const;
    virtual KCalCore::Attendee::Ptr currentAttendee() const = 0;
    virtual void updateCurrentItem() = 0;

    virtual void changeStatusForMe( KCalCore::Attendee::PartStat status ) = 0;

    virtual bool eventFilter( QObject *, QEvent * );

  protected slots:
    void addNewAttendee();
    void openAddressBook();

    void setEnableAttendeeInput( bool enabled );
    void updateAttendeeInput();
    void clearAttendeeInput();
    void fillAttendeeInput( KCalCore::Attendee::Ptr &a );
    void updateAttendee();

  protected:
    KPIM::AddresseeLineEdit *mNameEdit;
    QString mUid;
    KComboBox *mRoleCombo;
    QCheckBox *mRsvpButton;
    KComboBox *mStatusCombo;

    KHBox *mOrganizerHBox;
    KComboBox *mOrganizerCombo; // either we organize it (combo shown)
    QLabel *mOrganizerLabel; // or someone else does (just a label is shown)

    QLabel *mDelegateLabel;

    QPushButton *mAddButton;
    QPushButton *mRemoveButton;
    QPushButton *mAddressBookButton;

    QList<KCalCore::Attendee::Ptr> mDelAttendees;
    QList<KCalCore::Attendee::Ptr>mNewAttendees;

  private:
    bool mDisableItemUpdate;
};

#endif
