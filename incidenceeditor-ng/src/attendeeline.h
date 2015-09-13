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

#include <Libkdepim/AddresseeLineEdit>
#include <Libkdepim/MultiplyingLine>

#include <KCalCore/Attendee>

#include <QCheckBox>
#include <QToolButton>

class QKeyEvent;

namespace IncidenceEditorNG
{

class AttendeeData;

class AttendeeComboBox : public QToolButton
{
    Q_OBJECT
public:
    explicit AttendeeComboBox(QWidget *parent);

    void addItem(const QIcon &icon, const QString &text);
    void addItems(const QStringList &texts);

    int currentIndex() const;

Q_SIGNALS:
    void rightPressed();
    void leftPressed();
    void itemChanged();

public Q_SLOTS:
    /** Clears the combobox, removing all items. */
    void clear();
    void setCurrentIndex(int index);

protected:
    void keyPressEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotActionTriggered();

private:
    QMenu *mMenu;
    QVector<QPair<QString, QIcon> > mList;
    int mCurrentIndex;
};

class AttendeeLineEdit : public KPIM::AddresseeLineEdit
{
    Q_OBJECT
public:
    explicit AttendeeLineEdit(QWidget *parent);

Q_SIGNALS:
    void deleteMe();
    void leftPressed();
    void rightPressed();
    void upPressed();
    void downPressed();

protected:
    void keyPressEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
};

class AttendeeLine : public KPIM::MultiplyingLine
{
    Q_OBJECT
public:
    enum AttendeeActions {
        EventActions,
        TodoActions
    };

    explicit AttendeeLine(QWidget *parent);
    virtual ~AttendeeLine() {}

    void activate() Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;

    bool isEmpty() const Q_DECL_OVERRIDE;
    void clear() Q_DECL_OVERRIDE;

    bool isModified() const Q_DECL_OVERRIDE;
    void clearModified() Q_DECL_OVERRIDE;

    KPIM::MultiplyingLineData::Ptr data() const Q_DECL_OVERRIDE;
    void setData(const KPIM::MultiplyingLineData::Ptr &data) Q_DECL_OVERRIDE;

    void fixTabOrder(QWidget *previous) Q_DECL_OVERRIDE;
    QWidget *tabOut() const Q_DECL_OVERRIDE;

    void moveCompletionPopup() Q_DECL_OVERRIDE;
    void setCompletionMode(KCompletion::CompletionMode) Q_DECL_OVERRIDE;

    int setColumnWidth(int w) Q_DECL_OVERRIDE;

    void aboutToBeDeleted() Q_DECL_OVERRIDE;

    void setActions(AttendeeActions actions);

Q_SIGNALS:
    void changed();
    void changed(const KCalCore::Attendee::Ptr &oldAttendee,
                 const KCalCore::Attendee::Ptr &newAttendee);
    void editingFinished(KPIM::MultiplyingLine *);

private Q_SLOTS:
    void slotTextChanged(const QString &);
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
