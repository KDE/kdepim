/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2006, The KNotes Developers

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

#include <tqstring.h>
#include <tqevent.h>
#include <tqframe.h>
#include <tqpoint.h>
#include <tqcolor.h>

#include <kconfig.h>
#include <kxmlguiclient.h>

class TQLabel;

class KXMLGUIBuilder;

class KFind;
class KPopupMenu;
class KNoteButton;
class KNoteEdit;
class KNoteConfig;
class KToolBar;
class KListAction;
class KToggleAction;

namespace KCal {
    class Journal;
}


class KNote : public TQFrame, virtual public KXMLGUIClient
{
    Q_OBJECT
public:
    KNote( TQDomDocument buildDoc, KCal::Journal *journal, TQWidget *parent = 0,
           const char *name = 0 );
    ~KNote();

    void changeJournal(KCal::Journal *);
    void saveData( bool update = true);
    void saveConfig() const;

    TQString noteId() const;

    TQString name() const;
    TQString text() const;
    TQString plainText() const;

    void setName( const TQString& name );
    void setText( const TQString& text );

    TQColor fgColor() const;
    TQColor bgColor() const;
    void setColor( const TQColor& fg, const TQColor& bg );

    void find( const TQString& pattern, long options );

    bool isModified() const;

    void sync( const TQString& app );
    bool isNew( const TQString& app ) const;
    bool isModified( const TQString& app ) const;

    static void setStyle( int style );

    void deleteWhenIdle();
    void blockEmitDataChanged( bool _b ) { m_blockEmitDataChanged = _b;}
public slots:
    void slotKill( bool force = false );

signals:
    void sigRequestNewNote();
    void sigShowNextNote();
    void sigNameChanged();
    void sigDataChanged(const TQString &);
    void sigColorChanged();
    void sigKillNote( KCal::Journal* );

    void sigFindFinished();

protected:
    virtual void drawFrame( TQPainter* );
    virtual void showEvent( TQShowEvent* );
    virtual void resizeEvent( TQResizeEvent* );
    virtual void closeEvent( TQCloseEvent* );
    virtual void dropEvent( TQDropEvent* );
    virtual void dragEnterEvent( TQDragEnterEvent* );

    virtual bool event( TQEvent* );
    virtual bool eventFilter( TQObject*, TQEvent* );

    virtual bool focusNextPrevChild( bool );

    /// Protect against deletion while we are running a sub-eventloop
    void aboutToEnterEventLoop();
    void eventLoopLeft();

private slots:
    void slotRename();
    void slotUpdateReadOnly();
    void slotClose();

    void slotSend();
    void slotMail();
    void slotPrint();
    void slotSaveAs();

    void slotInsDate();
    void slotSetAlarm();

    void slotPreferences();
    void slotPopupActionToDesktop( int id );

    void slotFindNext();
    void slotHighlight( const TQString& txt, int idx, int len );

    void slotApplyConfig();
    void slotUpdateKeepAboveBelow();
    void slotUpdateShowInTaskbar();
    void slotUpdateDesktopActions();

    void slotUpdateViewport( int, int );
    void slotRequestNewNote();
    void slotSaveData();
private:
    void updateFocus();
    void updateMask();
    void updateLayout();
    void updateLabelAlignment();
    void updateBackground( int offset = -1 );

    void createFold();

    void toDesktop( int desktop );

    TQString toPlainText( const TQString& );

private:
    TQLabel        *m_label, *m_pushpin, *m_fold;
    KNoteButton   *m_button;
    KToolBar      *m_tool;
    KNoteEdit     *m_editor;

    KNoteConfig   *m_config;
    KCal::Journal *m_journal;

    KFind         *m_find;

    KPopupMenu    *m_menu;
    KPopupMenu    *m_edit_menu;

    KToggleAction *m_readOnly;

    KListAction   *m_toDesktop;
    KToggleAction *m_keepAbove;
    KToggleAction *m_keepBelow;

    KSharedConfig::Ptr m_kwinConf;

    static int s_ppOffset;

    int m_busy;
    bool m_deleteWhenIdle;
    bool m_blockEmitDataChanged;
};

#endif
