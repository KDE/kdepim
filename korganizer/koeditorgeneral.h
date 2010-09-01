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
#include <tqlineedit.h>

class TQWidget;
class TQBoxLayout;
class TQHBox;
class TQLineEdit;
class TQLabel;
class TQCheckBox;
class TQSpinBox;
class TQPushButton;
class TQComboBox;
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
    FocusLineEdit( TQWidget *parent );

  signals:
    void focusReceivedSignal();

  protected:
    void focusInEvent ( TQFocusEvent *e );

  private:
    bool mSkipFirst;
};

class KOEditorGeneral : public QObject
{
    Q_OBJECT
  public:
    KOEditorGeneral (TQObject* parent=0,const char* name=0);
    virtual ~KOEditorGeneral();

    void initHeader( TQWidget *parent,TQBoxLayout *topLayout );
    void initDescription(TQWidget *,TQBoxLayout *);
    void initSecrecy(TQWidget *,TQBoxLayout *);
    void initAlarm(TQWidget *,TQBoxLayout *);
    void initAttachments(TQWidget *,TQBoxLayout *);

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

    void setSummary( const TQString & );
    void setDescription( const TQString & );

    TQObject *typeAheadReceiver() const;

  public slots:
    void setCategories(const TQStringList &categories);
    void selectCategories();
    void setType( const TQCString &type );
    void addAttachments( const TQStringList &attachments,
                         const TQStringList& mimeTypes = TQStringList(),
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
    TQLineEdit               *mSummaryEdit;
    TQLineEdit               *mLocationEdit;
    TQLabel                  *mAttendeeSummaryLabel;
    TQLabel                  *mRecEditLabel;
    TQPushButton             *mRecEditButton;
    TQLabel                  *mAlarmBell;
    TQLabel                  *mAlarmInfoLabel;
    TQCheckBox               *mAlarmButton;
    TQSpinBox                *mAlarmTimeEdit;
    TQComboBox               *mAlarmIncrCombo;
    TQPushButton             *mAlarmAdvancedButton;
    KTextEdit               *mDescriptionEdit;
    TQLabel                  *mOwnerLabel;
    TQComboBox               *mSecrecyCombo;
    TQPushButton             *mCategoriesButton;
    KSqueezedTextLabel      *mCategoriesLabel;
    KOEditorAttachments     *mAttachments;
    TQLabel                  *mResourceLabel;

  private:
    Alarm *alarmFromSimplePage( Incidence *incidence ) const;
    bool isSimpleAlarm( Alarm *alarm ) const;

    bool mAlarmIsSimple;
    TQHBox *mSimpleAlarmBox;
    TQStringList mCategories;
    TQCString mType; // as in Incidence::type()
    KCal::Alarm::List mAlarmList;
};

#endif
