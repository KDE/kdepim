/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef INCIDENCEEDITOR_ATTENDEELINE_H
#define INCIDENCEEDITOR_ATTENDEELINE_H

#include "incidenceeditors-ng_export.h"

#include <libkdepim/addressline/addresseelineedit.h>
#include <libkdepim/multiplyingline/multiplyingline.h>

#include <KCalCore/Attendee>

#include <QCheckBox>
#include <QToolButton>

class QKeyEvent;

namespace IncidenceEditorNG {

class AttendeeData;

class INCIDENCEEDITORS_NG_EXPORT AttendeeComboBox : public QToolButton
{
  Q_OBJECT
  public:
    explicit AttendeeComboBox( QWidget *parent );

    void addItem( const QIcon &icon, const QString &text );
    void addItems( const QStringList &texts );

    int currentIndex() const;

  signals:
    void rightPressed();
    void leftPressed();
    void itemChanged();

  public slots:
    /** Clears the combobox, removing all items. */
    void clear();
    void setCurrentIndex( int index );

  protected:
    void keyPressEvent( QKeyEvent *ev );

  private slots:
    void slotActionTriggered();

  private:
    QMenu *mMenu;
    QList<QPair<QString, QIcon> > mList;
    int mCurrentIndex;
};

class INCIDENCEEDITORS_NG_EXPORT AttendeeLineEdit : public KPIM::AddresseeLineEdit
{
  Q_OBJECT
  public:
    explicit AttendeeLineEdit( QWidget * parent );

  signals:
    void deleteMe();
    void leftPressed();
    void rightPressed();
    void upPressed();
    void downPressed();

  protected:
    void keyPressEvent( QKeyEvent *ev );
};

class INCIDENCEEDITORS_NG_EXPORT AttendeeLine : public KPIM::MultiplyingLine
{
  Q_OBJECT
  public:
    enum AttendeeActions {
      EventActions,
      TodoActions
    };

    explicit AttendeeLine( QWidget *parent );
    virtual ~AttendeeLine(){}

    virtual void activate();
    virtual bool isActive() const;

    virtual bool isEmpty() const;
    virtual void clear();

    virtual bool isModified() const;
    virtual void clearModified();

    virtual KPIM::MultiplyingLineData::Ptr data() const;
    virtual void setData( const KPIM::MultiplyingLineData::Ptr &data );

    virtual void fixTabOrder( QWidget *previous );
    virtual QWidget *tabOut() const;

    virtual void moveCompletionPopup();
    virtual void setCompletionMode( KGlobalSettings::Completion );

    virtual int setColumnWidth( int w );

    virtual void aboutToBeDeleted();

    void setActions( AttendeeActions actions );

  signals:
    void changed();
    void changed( const KCalCore::Attendee::Ptr &oldAttendee,
                  const KCalCore::Attendee::Ptr &newAttendee );
    void editingFinished( KPIM::MultiplyingLine * );

  private slots:
    void slotTextChanged( const QString & );
    void slotHandleChange();
    void slotComboChanged();

  private:
    void dataFromFields();
    void fieldsFromData();

    AttendeeComboBox *mRoleCombo;
    AttendeeComboBox *mStateCombo;
    AttendeeComboBox *mResponseCombo;
    AttendeeLineEdit *mEdit;
    QSharedPointer<AttendeeData> mData;
    QString mUid;
    bool mModified;
};

}

#endif
