/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#ifndef INCIDENCEEDITOR_INCIDENCEDIALOG_H
#define INCIDENCEEDITOR_INCIDENCEDIALOG_H

#include "incidenceeditors_ng_export.h"
#include "editoritemmanager.h"

#include <KDialog>

namespace Akonadi {
  class IncidenceChanger;
}

namespace IncidenceEditorNG {

class IncidenceDialogPrivate;

class INCIDENCEEDITORS_NG_EXPORT IncidenceDialog : public KDialog
{
  Q_OBJECT
  public:
    explicit IncidenceDialog( Akonadi::IncidenceChanger *changer = 0,
                              QWidget *parent = 0, Qt::WindowFlags flags = 0 );
    ~IncidenceDialog();

    /**
     * Loads the @param item into the dialog.
     *
     * To create a new Incidence pass an invalid item with either an
     * KCalCore::Event:Ptr or a KCalCore::Todo:Ptr set as payload. Note: When the
     * item is invalid, i.e. it has an invalid id, a valid payload <em>must</em>
     * be set.
     *
     * When the item has is valid this method will fetch the payload when this is
     * not already set.
     */
    virtual void load( const Akonadi::Item &item, const QDate &activeDate = QDate() );

    /**
     * Sets the Collection combobox to @param collection.
     */
    virtual void selectCollection( const Akonadi::Collection &collection );

    virtual void setIsCounterProposal( bool isCounterProposal );

    /**
      Returns the object that will receive all key events.
    */
    QObject *typeAheadReceiver() const;

    /**
       By default, if you load an incidence into the editor ( load(item) ), then press [OK]
       without changing anything, the dialog is dismissed, and the incidence isn't saved
       to akonadi.

       Call this method with @p initiallyDirty = true if you want the incidence to be saved,
       It's useful if you're creating a dialog with an already crafted content, like in kmail's
       "Create Todo/Reminder Feature".
    */
    void setInitiallyDirty( bool initiallyDirty );

    Akonadi::Item item() const;

Q_SIGNALS:
    /**
     * This signal is emitted when an incidence is created.
     * @param collection The collection where it was created.
     */
    void incidenceCreated( const Akonadi::Item & );
    void invalidCollection() const;
  protected:
    virtual void closeEvent( QCloseEvent *event );

  protected Q_SLOTS:
    virtual void slotButtonClicked( int button );
    void handleSelectedCollectionChange( const Akonadi::Collection &collection );

  private:
    IncidenceDialogPrivate *const d_ptr;
    Q_DECLARE_PRIVATE( IncidenceDialog )
    Q_DISABLE_COPY( IncidenceDialog )

    void writeConfig();
    void readConfig();

    Q_PRIVATE_SLOT( d_ptr, void handleAlarmCountChange(int) )
    Q_PRIVATE_SLOT( d_ptr, void handleItemSaveFinish(IncidenceEditorNG::EditorItemManager::SaveAction) )
    Q_PRIVATE_SLOT( d_ptr, void handleItemSaveFail(IncidenceEditorNG::EditorItemManager::SaveAction,QString) )
    Q_PRIVATE_SLOT( d_ptr, void handleRecurrenceChange(IncidenceEditorNG::RecurrenceType) )
    Q_PRIVATE_SLOT( d_ptr, void loadTemplate(QString) )
    Q_PRIVATE_SLOT( d_ptr, void saveTemplate(QString) )
    Q_PRIVATE_SLOT( d_ptr, void storeTemplatesInConfig(QStringList) )
    Q_PRIVATE_SLOT( d_ptr, void updateAttachmentCount(int) )
    Q_PRIVATE_SLOT( d_ptr, void updateAttendeeCount(int) )
    Q_PRIVATE_SLOT( d_ptr, void updateButtonStatus(bool) )
    Q_PRIVATE_SLOT( d_ptr, void showMessage(QString,KMessageWidget::MessageType) )
    Q_PRIVATE_SLOT( d_ptr, void slotInvalidCollection() )
};

}

#endif
