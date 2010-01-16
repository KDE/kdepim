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

#include <QSplitter>

class KComboBox;
class KNComposerEditor;
class QGroupBox;
namespace KPIMIdentities {
  class IdentityCombo;
}

/** Message composer view. */
class KNComposer::ComposerView  : public QSplitter {

  Q_OBJECT

  public:
    /**
      Constructor.
    */
    explicit ComposerView( KNComposer *_composer );
    /**
      Destructor.
    */
    virtual ~ComposerView();

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


    KNLineEditSpell *s_ubject;

    KNLineEdit  *g_roups;
    KNLineEdit  *t_o;

    KComboBox   *f_up2;

    KNComposerEditor    *e_dit;
    QPushButton *c_ancelEditorBtn;

    QWidget         *a_ttWidget;
    AttachmentView  *a_ttView;
    QPushButton     *a_ttAddBtn,
                    *a_ttRemoveBtn,
                    *a_ttEditBtn;
    bool v_iewOpen;

  public slots:
    /**
      Appends the signature to the editor.
    */
    void appendSignature();

  private slots:
    /**
      Called when a new identity is selected to update edit lines that
      contain identity information.
    */
    void slotIdentityChanged( uint uoid );

  private:
    KPIMIdentities::IdentityCombo *mIdentitySelector;
    QLabel *mIdentitySelectorLabel;

    QList<QWidget*> mEdtList;

    QLabel      *l_to,
                *l_groups,
                *l_fup2;
    QPushButton *g_roupsBtn,
                *t_oBtn;
    QGroupBox  *n_otification;
};

#endif
