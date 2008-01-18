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

class KNLineEditSpell;
class KNLineEdit;
class KNComposer;
class KNComposerEditor;

/** Message composer view. */
class KNComposer::ComposerView  : public QSplitter {

  public:
    ComposerView( KNComposer *_composer );
    ~ComposerView();
    void focusNextPrevEdit(const QWidget* aCur, bool aNext);
    void setMessageMode(KNComposer::MessageMode mode);
    void showAttachmentView();
    void hideAttachmentView();
    void showExternalNotification();
    void hideExternalNotification();
    QList<QWidget*> mEdtList;

    QLabel      *l_to,
                *l_groups,
                *l_fup2;
    KNLineEditSpell *s_ubject;

    KNLineEdit  *g_roups;
    KNLineEdit  *t_o;

    KComboBox   *f_up2;
    QPushButton *g_roupsBtn,
                *t_oBtn;

    KNComposerEditor    *e_dit;
    QGroupBox  *n_otification;
    QPushButton *c_ancelEditorBtn;

    QWidget         *a_ttWidget;
    AttachmentView  *a_ttView;
    QPushButton     *a_ttAddBtn,
                    *a_ttRemoveBtn,
                    *a_ttEditBtn;
    bool v_iewOpen;
};

#endif
