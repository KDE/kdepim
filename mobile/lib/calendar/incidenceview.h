/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

#ifndef INCIDENCEVIEW_H
#define INCIDENCEVIEW_H

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <kcalcore/incidence.h>

#include <incidenceeditor-ng/combinedincidenceeditor.h>
#include <incidenceeditor-ng/editoritemmanager.h>

#include "kdeclarativefullscreenview.h"

class KDateComboBox;
class KJob;
class KTimeComboBox;
class QTimeEdit;
class QDateEdit;

namespace Akonadi {
class CollectionComboBox;
}

namespace IncidenceEditorNG {
class IncidenceAttendee;
class IncidenceDateTime;
}

class MobileIncidenceGeneral;
class MobileIncidenceMore;

class IncidenceView : public KDeclarativeFullScreenView, public IncidenceEditorNG::ItemEditorUi
{
  Q_OBJECT
  public:
    explicit IncidenceView( QWidget* parent = 0 );
    ~IncidenceView();

    void load( const Akonadi::Item &item, const QDate &date );

    void setCollectionCombo( Akonadi::CollectionComboBox * );
    void setGeneralEditor( MobileIncidenceGeneral * );
    void setMoreEditor( MobileIncidenceMore * );

    void setDefaultCollection( const Akonadi::Collection &collection );
    void setIsCounterProposal( bool isCounterProposal );

  public: /// ItemEditorUi function implementations
    virtual bool containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const;
    virtual bool hasSupportedPayload( const Akonadi::Item &item ) const;
    virtual bool isDirty() const;
    virtual bool isValid() const;
    virtual void load( const Akonadi::Item &item );
    virtual Akonadi::Item save( const Akonadi::Item &item );
    virtual Akonadi::Collection selectedCollection() const;
    virtual void reject( RejectReason reason, const QString &errorMessage = QString() );

  signals:
    void showCalendarWidget( int day, int month, int year );
    void showClockWidget( int hour, int minute );

  public slots:
    void save();   /// Ok clicked in the user interface
    void cancel(); /// Cancel clicked in the user interface
    void showCalendar( QObject *obj );
    void setNewDate( int day, int month, int year );
    void showClock( QObject *obj );
    void setNewTime( int hour, int minute );

  private slots:
    void slotSaveFinished( IncidenceEditorNG::EditorItemManager::SaveAction action );
    void slotSaveFailed( IncidenceEditorNG::EditorItemManager::SaveAction action,
                         const QString &message );

  private:
    void doDelayedInit();
    void initIncidenceMore();

  private:
    QDate mActiveDate;
    Akonadi::Item mItem;
    IncidenceEditorNG::EditorItemManager *mItemManager;
    Akonadi::CollectionComboBox *mCollectionCombo;
    Akonadi::Collection mDefaultCollection;
    IncidenceEditorNG::CombinedIncidenceEditor *mEditor;
    IncidenceEditorNG::IncidenceDateTime *mEditorDateTime;

    /// We need this because we can't rely on the order in which those two are added.
    MobileIncidenceMore *mIncidenceMore;
    MobileIncidenceGeneral *mIncidenceGeneral;

    KDateComboBox *mDateWidget;
    KTimeComboBox *mTimeWidget;
    IncidenceEditorNG::IncidenceAttendee *mIncidenceAttendee;
};

#endif // INCIDENCEVIEW_H
