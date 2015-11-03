/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2013, The KNotes Developers

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************/

#ifndef KNOTE_H
#define KNOTE_H
#include "config-kdepim.h"
#include <QDomDocument>
#include <QEvent>
#include <QFrame>

#include <kconfig.h>
#include <kxmlguiclient.h>
#include <KSharedConfig>
#include <AkonadiCore/Item>
#include "knoteinterface.h"

class KNoteDisplaySettings;

class QLabel;
class QLayout;
class QSizeGrip;

class QMenu;
class KNoteButton;
class KNoteEdit;
class KSelectAction;
class KToggleAction;
class KToolBar;
class KJob;

class KNote : public QFrame, virtual public KXMLGUIClient, public KNoteInterface
{
    Q_OBJECT
public:
    explicit KNote(const QDomDocument &buildDoc, const Akonadi::Item &item, bool allowDebugBaloo = false, QWidget *parent = Q_NULLPTR);
    ~KNote();

    void setChangeItem(const Akonadi::Item &item, const QSet<QByteArray> &set = QSet<QByteArray>());
    void saveNote(bool force = false, bool sync = false);

    QString name() const;
    QString text() const;
    Akonadi::Item::Id noteId() const;

    Akonadi::Item item() const;

    void setName(const QString &name);
    void setText(const QString &text);

    bool isModified() const;
    bool isDesktopAssigned() const;

    void toDesktop(int desktop);

public Q_SLOTS:
    void slotRename();
    void slotKill(bool force = false);
    void slotClose();

Q_SIGNALS:
    void sigRequestNewNote();
    void sigShowNextNote();
    void sigNameChanged(const QString &);
    void sigColorChanged();
    void sigKillNote(Akonadi::Item::Id);

protected:
    void contextMenuEvent(QContextMenuEvent *) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *) Q_DECL_OVERRIDE;

    bool event(QEvent *) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotUpdateReadOnly();

    void slotSend();
    void slotMail();
    void slotPrint();
    void slotPrintPreview();
    void slotSaveAs();

    void slotSetAlarm();

    void slotPreferences();
    void slotPopupActionToDesktop(QAction *act);

    void slotApplyConfig();

    void slotUpdateShowInTaskbar();
    void slotUpdateDesktopActions();

    void slotKeepAbove();
    void slotKeepBelow();

    void slotRequestNewNote();
    void slotNoteSaved(KJob *job);
    void slotDebugBaloo();

private:
    void updateKeepAboveBelow(bool save = true);
    void buildGui();
    void createActions();
    void createNoteEditor(const QString &configFile);
    void createNoteFooter();
    void createNoteHeader();
    void prepare();

    void updateFocus();
    void updateLayout();
    void updateLabelAlignment();

    void setColor(const QColor &, const QColor &);
    void print(bool preview);
    void setDisplayDefaultValue();

private:
    void loadNoteContent(const Akonadi::Item &item);
    void updateAllAttributes();
    void saveNoteContent();
    Akonadi::Item mItem;
    QLayout       *m_noteLayout;
    QLabel        *m_label;
    QSizeGrip     *m_grip;
    KNoteButton   *m_button;
    KToolBar      *m_tool;
    KNoteEdit     *m_editor;

    QMenu         *m_menu;

    KToggleAction *m_readOnly;

#if KDEPIM_HAVE_X11
    KSelectAction   *m_toDesktop;
#endif
    KToggleAction *m_keepAbove;
    KToggleAction *m_keepBelow;

    KSharedConfig::Ptr m_kwinConf;

    KNoteDisplaySettings *mDisplayAttribute;
    bool mAllowDebugBaloo;
};

#endif
