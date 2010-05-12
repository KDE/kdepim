/*
    This file is part of KOrganizer.

    Copyright (C) 2010  Bertjan Broeksema <b.broeksema@home.nl>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef INCIDENCEGENERALEDITOR_H
#define INCIDENCEGENERALEDITOR_H

#include <QtCore/QDateTime>
#include <QtGui/QWidget>

#include <KDateTime>

class QDateTime;

namespace KCal {
class ICalTimeZones;
}

namespace KPIM {
class KTimeZoneComboBox;
}

namespace Ui {
class IncidenceGeneral;
}

class IncidenceGeneralEditor : public QWidget
{
  Q_OBJECT
  public:
    ~IncidenceGeneralEditor();

  public slots:
    void setDuration();

  signals:
    void dateTimeStrChanged( const QString &dateTimeStr );
    
  protected: /// Methods
    /**
     * Disable creation of plain IncidenceGeneralEditor widgets. Use one of the
     * sub-classes instead.
     */
    explicit IncidenceGeneralEditor( QWidget *parent = 0 );

    void emitDateTimeStr();

  private slots:
    void enableRichTextDescription( bool enable );
    
  protected slots:
    virtual void enableTimeEditors( bool enabled ) = 0;
    virtual void slotHasTimeCheckboxToggled( bool checked );

  private:
    void initDescriptionToolBar();
    
  protected: /// Members
    KCal::ICalTimeZones  *mTimeZones;
    Ui::IncidenceGeneral *mUi;

    // current start and end date and time
    QDateTime mCurrStartDateTime;
    QDateTime mCurrEndDateTime;

    // specs
    KDateTime::Spec mStartSpec;
    KDateTime::Spec mEndSpec;
};

class EventGeneralEditor : public IncidenceGeneralEditor
{
  Q_OBJECT
  public:
    explicit EventGeneralEditor( QWidget *parent = 0 );

  signals:
    void allDayChanged( bool changed );


  private slots:
    virtual void slotHasTimeCheckboxToggled( bool checked );

  private:
    void initTextEditToolBar();
    void enableTimeEditors( bool enabled );

};

class TodoGeneralEditor : public IncidenceGeneralEditor
{
  Q_OBJECT
  public:
    explicit TodoGeneralEditor( QWidget *parent = 0 );

  private slots:
    void enableTimeEditors( bool enabled );
    void enableStartEdit( bool enable );
    void enableEndEdit( bool enable );
    void updateHasTimeCheckBox();

  private:
    void enableTimeEdit( QWidget *dateEdit,
                         QWidget *timeEdit,
                         KPIM::KTimeZoneComboBox *timeZoneCmb,
                         bool enable );
    
    
  private:

};

#endif // INCIDENCEGENERALEDITOR_H
