/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOEDITORGENERAL_H
#define KOEDITORGENERAL_H

#include <libkcal/alarm.h>
#include <qlineedit.h>

class QWidget;
class QBoxLayout;
class QHBox;
class QLineEdit;
class QLabel;
class QCheckBox;
class QSpinBox;
class QPushButton;
class QComboBox;
class KTextEdit;
class KSqueezedTextLabel;
class KURL;
class KOEditorAttachments;

namespace KCal {
  class Incidence;
  class Calendar;
}
using namespace KCal;

class FocusLineEdit : public QLineEdit
{
    Q_OBJECT
  public:
    FocusLineEdit( QWidget *parent );

  signals:
    void focusReceivedSignal();

  protected:
    void focusInEvent ( QFocusEvent *e );

  private:
    bool mSkipFirst;
};

class KOEditorGeneral : public QObject
{
    Q_OBJECT
  public:
    KOEditorGeneral (QObject* parent=0,const char* name=0);
    virtual ~KOEditorGeneral();

    void initHeader( QWidget *parent,QBoxLayout *topLayout );
    void initDescription(QWidget *,QBoxLayout *);
    void initSecrecy(QWidget *,QBoxLayout *);
    void initAlarm(QWidget *,QBoxLayout *);
    void initAttachments(QWidget *,QBoxLayout *);

    /** Set widgets to default values */
    void setDefaults(bool allDay);
    /** Read incidence object and setup widgets accordingly */
    void readIncidence( Incidence *incidence, Calendar *calendar );
    /** Write incidence settings to incidence object */
    void writeIncidence( Incidence *incidence );

    /** Check if the input is valid. */
    bool validateInput() { return true; }

    void enableAlarm( bool enable );
    void toggleAlarm( bool on );

    void setSummary( const QString & );
    void setDescription( const QString & );

    QObject *typeAheadReceiver() const;

  public slots:
    void setCategories(const QStringList &categories);
    void selectCategories();
    void setType( const QCString &type );
    void addAttachments( const QStringList &attachments,
                         const QStringList& mimeTypes = QStringList(),
                         bool inlineAttachment = false );

  protected slots:
    void editAlarms();
    void updateAlarmWidgets( Incidence *incidence );
    void updateDefaultAlarmTime();
    void updateAttendeeSummary( int count );

  signals:
    void openCategoryDialog();
    void updateCategoryConfig();
    void focusReceivedSignal();
    void openURL( const KURL & );

  protected:
    QLineEdit               *mSummaryEdit;
    QLineEdit               *mLocationEdit;
    QLabel                  *mAttendeeSummaryLabel;
    QLabel                  *mRecEditLabel;
    QPushButton             *mRecEditButton;
    QLabel                  *mAlarmBell;
    QLabel                  *mAlarmInfoLabel;
    QCheckBox               *mAlarmButton;
    QSpinBox                *mAlarmTimeEdit;
    QComboBox               *mAlarmIncrCombo;
    QPushButton             *mAlarmAdvancedButton;
    KTextEdit               *mDescriptionEdit;
    QLabel                  *mOwnerLabel;
    QComboBox               *mSecrecyCombo;
    QPushButton             *mCategoriesButton;
    KSqueezedTextLabel      *mCategoriesLabel;
    KOEditorAttachments     *mAttachments;
    QLabel                  *mResourceLabel;

  private:
    Alarm *alarmFromSimplePage( Incidence *incidence ) const;
    bool isSimpleAlarm( Alarm *alarm ) const;

    bool mAlarmIsSimple;
    QHBox *mSimpleAlarmBox;
    QStringList mCategories;
    QCString mType; // as in Incidence::type()
    KCal::Alarm::List mAlarmList;
};

#endif