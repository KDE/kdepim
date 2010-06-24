/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

#include <Akonadi/Item>
#include <KCal/Incidence>

#include <incidenceeditors/incidenceeditor-ng/combinedincidenceeditor.h>
#include <incidenceeditors/incidenceeditor-ng/editoritemmanager.h>

#include "kdeclarativefullscreenview.h"

class KJob;

namespace Akonadi {
class CollectionComboBox;
}

namespace IncidenceEditorsNG {
class IncidenceDateTime;
}

class MobileIncidenceGeneral;
class MobileIncidenceMore;

class IncidenceView : public KDeclarativeFullScreenView, public Akonadi::ItemEditorUi
{
  Q_OBJECT;
  public:
    explicit IncidenceView( QWidget* parent = 0 );
    ~IncidenceView();

    void load( const Akonadi::Item &item, const QDate &date );

    void setCollectionCombo( Akonadi::CollectionComboBox * );
    void setGeneralEditor( MobileIncidenceGeneral * );
    void setMoreEditor( MobileIncidenceMore * );

  public: /// ItemEditorUi function implementations
    virtual bool containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const;
    virtual bool hasSupportedPayload( const Akonadi::Item &item ) const;
    virtual bool isDirty() const;
    virtual bool isValid();
    virtual void load( const Akonadi::Item &item );
    virtual Akonadi::Item save( const Akonadi::Item &item );
    virtual Akonadi::Collection selectedCollection() const;
    virtual void reject( RejectReason reason, const QString &errorMessage = QString() );

  public slots:
    void save();   /// Ok clicked in the user interface
    void cancel(); /// Cancel clicked in the user interface

  private slots:
    void slotSaveFinished();
    void slotSaveFailed( const QString &message );

  private:
    QDate mActiveDate;
    Akonadi::Item mItem;
    Akonadi::EditorItemManager *mItemManager;
    Akonadi::CollectionComboBox *mCollectionCombo;
    IncidenceEditorsNG::CombinedIncidenceEditor *mEditor;
    IncidenceEditorsNG::IncidenceDateTime *mEditorDateTime;
};

#endif // INCIDENCEVIEW_H
