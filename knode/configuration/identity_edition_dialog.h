/*
  Copyright 2009 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#ifndef KNODE_IDENTITY_EDITION_DIALOG_H
#define KNODE_IDENTITY_EDITION_DIALOG_H

#include "knode_export.h"
#include "ui_identity_edition_dialog.h"

#include <QList>
#include <QPointer>


namespace KNode {

class IdentityNameEditPrivate;

/**
  Edition dialog for identities.
*/
class KNODE_EXPORT IdentityEditionDialog : public KDialog, private Ui::IdentityEditionDialog
{
  Q_OBJECT

  public:
    /**
      Contruct a new dialog to edit identities.
      @param uoid the UOID of the identity to select by default this dialog is opened.
      @param parent the parent widget.
    */
    explicit IdentityEditionDialog( uint uoid, QWidget *parent = 0 );
    /**
      Destructor.
    */
    ~IdentityEditionDialog();

  protected slots:
    /**
      Handles button clicks.
      Reimplemented from KDialog::slotButtonClicked().
    */
    virtual void slotButtonClicked( int button );

  private:
    /**
      (Re)loads this widget with information from the IdentityManager.
    */
    void reload();

    /**
      Copy field from this widget into an identity.
      @param uoid identifier of the target identity.
    */
    void saveIntoIdentity( uint uoid ) const;
    /**
      Loads an identity into this widget.
      @param uoid Identifier of an @em existing identity.
    */
    void loadFromIdentity( uint uoid );

    /**
      Sets the currently selected identity without emitting any signals.
      @param uoid Identifier of an identity. If this identity does not
      exist a random identity is selected.
    */
    void setCurrentIdentity( uint uoid );

  private slots:
    /**
      Called by the currentIndexChanged() of the combo box.
      It saves the previously selected identity and the loads the widget with
      information from the current selection.
      @param index index of the selection in the combo box.
    */
    void identitySelected( int index );

    /**
      Starts the creation of a new identity.

      Actual saving will be made when the user validate this dialog.
    */
    void createNewIdentity();

    /**
      Create a new identity prefilled with the currently selected
      identity information.
    */
    void duplicateCurrentIdentity();

    /**
      Make the combo box editable to allow renaming of the selected identity.
    */
    void startIdentityRenaming();
    /**
      Make the combo box uneditable.
    */
    void stopIdentityRenaming();
    /**
      Called when the name of an identity stopped being edited.
      This also call stopIdentityRenaming().
    */
    void changeIdentityName( const QString &newName );

    /**
      Deletes the currently selected identity.
    */
    void deleteCurrentIdentity();

  private:
    /**
      Identifier of the last selected identity (used to save
      modifications of an identity when the selected identity changed).
      -1 when there is no last selected entry (dialog opening, deletion).
    */
    int mCurrentIdentityUoid;

    /**
      List of modified identities. The index in this list is kept in sync
      with the index of the combo box mIdentitySelector.
    */
    QList<uint> mUoids;

    /**
      KLineEdit put into the combo box.
    */
    QPointer<IdentityNameEditPrivate> mIdentityNameEdit;
};



/**
  Special line edit to catch reliably the change of
  an identity name.
*/
class IdentityNameEditPrivate : public KLineEdit
{
  Q_OBJECT

  friend class IdentityEditionDialog;

  public:
    virtual ~IdentityNameEditPrivate()
    {
    }

  signals:
    /**
      Emitted when edition end (this widget lose the focus
      or returned is pressed).
      @param newName the text of this line edit.
    */
    void identityNameChanged( const QString &newName );

  protected:
    /**
      Reimplement to emit the identityNameChanged() signal
      on focus lose.
    */
    virtual void focusOutEvent( QFocusEvent *event )
    {
      emit identityNameChanged( text() );
      KLineEdit::focusOutEvent( event );
    }

  private:
    IdentityNameEditPrivate()
      : KLineEdit( 0 ) // null parent because KComboBox::setLineEdit() will take ownership
    {
      connect( this, SIGNAL(returnPressed(QString)),
               this, SIGNAL(identityNameChanged(QString)) );
    }

};




} // namespace KNode


#endif // KNODE_IDENTITY_EDITION_DIALOG_H
