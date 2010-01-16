/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2007 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/
#ifndef KNCOMPOSERVIEW_H
#define KNCOMPOSERVIEW_H

#include "kncomposer.h"
#include "ui_composer_view.h"

#include <QSplitter>

class KComboBox;
class KNComposerEditor;
class QGroupBox;
namespace KPIMIdentities {
  class IdentityCombo;
}

namespace KNode {
namespace Composer {

/** Message composer view. */
class View : public QSplitter, private Ui::View
{
  Q_OBJECT

  public:
    /**
      Constructor.
      @see completeSetup()
    */
    explicit View( KNComposer *_composer );
    /**
      Destructor.
    */
    virtual ~View();

    /**
      Completes the setup of this view.

      This must be call once after all setter methods (for identity, groups,
      subject, etc.) have all been called.
      @param firstEdit this indicates if this is the first edition of the post.
      @param mode the mode of this message.
    */
    void completeSetup( bool firstEdit, KNComposer::MessageMode mode );


    /**
      Gives the focus to the next/previous edition widget (group, to, subject, body, etc.)
      @param aCur the widget which currently have the focus
      @param aNext if true, the next widget get the focus., otherwise the previous one get the focus.
    */
    void focusNextPrevEdit(const QWidget* aCur, bool aNext);

    /**
      Set the message mode to @param mode.
    */
    void setMessageMode(KNComposer::MessageMode mode);

    /**
      Changes the font used in edition widget within this ComposerView.
    */
    void setComposingFont( const QFont &font );

    void showAttachmentView();
    void hideAttachmentView();
    void showExternalNotification();
    void hideExternalNotification();


    // Editor accessors
    /**
      Return the UOID of the selected identity.
    */
    uint selectedIdentity() const;
    /**
      Changes the selected identity and update related edit lines.
    */
    void setIdentity( uint uoid );

    /**
      Returns the sender full name and email address to use in a From: header of a message.
    */
    const QString from();
    /**
      Set the name and email address of the sender of the message.
    */
    void setFrom( const QString &from );

    /**
      Returns the followup-to list of groups (name are trimmed).
    */
    const QStringList groups() const;
    /**
      Sets the group list as a string (must be coma separated).
    */
    void setGroups( const QString &groups );

    /**
      Returns the email recipient of this message (as type by the user in the To: field).
    */
    const QString emailRecipient() const;
    /**
      Sets the email recipient list as a string.
      @param to a coma seperated list of recipient address.
    */
    void setEmailRecipient( const QString &to );

    /**
      Returns the followup-to list of groups (name are trimmed).
    */
    const QStringList followupTo() const;
    /**
      Sets the followup-to list as a string (must be a coma separated list of groups).
    */
    void setFollowupTo( const QString &followupTo );

    /**
      Returns the subject text.
    */
    const QString subject() const;
    /**
      Sets the subject.
    */
    void setSubject( const QString &subject );

    /**
      Returns the main text editor.
    */
    KNComposerEditor * editor() const;


    QWidget         *a_ttWidget;
    KNComposer::AttachmentView *a_ttView;
    QPushButton     *a_ttRemoveBtn,
                    *a_ttEditBtn;
    bool v_iewOpen;

  public slots:
    /**
      Appends the signature to the editor.
    */
    void appendSignature();

  signals:
    /**
      This signal is emitted when the user request the external body editor to
      be closed.
    */
    void closeExternalEditor();

  private slots:
    /**
      Called when a new identity is selected to update edit lines that
      contain identity information.
    */
    void slotIdentityChanged( uint uoid );

    /**
      Called when the content of the groups line edit changes.
    */
    void slotGroupsChanged( const QString &groupText );

  private:
    QList<QWidget*> mEdtList;

    QPushButton *a_ttAddBtn;

    /**
      Shows/hide the identity selector.
    */
    void showIdentity( bool show );
    /**
      Shows/hide the From editor.
    */
    void showFrom( bool show );
    /**
      Shows/hide the To editor.
    */
    void showTo( bool show );
    /**
      Shows/hide the Groups editor.
    */
    void showGroups( bool show );
    /**
      Shows/hide the Followup-To selector.
    */
    void showFollowupto( bool show );
    /**
      Shows/hide the Subject editor..
    */
    void showSubject( bool show );
};


} // namespace Composer
} // namespace KNode

#endif
