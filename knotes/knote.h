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

#include <qstring.h>
#include <qevent.h>
#include <qframe.h>
#include <qpoint.h>
#include <qcolor.h>

#include <kconfig.h>
#include <kxmlguiclient.h>

class QLabel;

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


class KNote : public QFrame, virtual public KXMLGUIClient
{
    Q_OBJECT
public:
    KNote( QDomDocument buildDoc, KCal::Journal *journal, QWidget *parent = 0,
           const char *name = 0 );
    ~KNote();

    void saveData();
    void saveConfig() const;

    QString noteId() const;

    QString name() const;
    QString text() const;
    QString plainText() const;

    void setName( const QString& name );
    void setText( const QString& text );

    QColor fgColor() const;
    QColor bgColor() const;
    void setColor( const QColor& fg, const QColor& bg );

    void find( const QString& pattern, long options );

    bool isModified() const;

    void sync( const QString& app );
    bool isNew( const QString& app ) const;
    bool isModified( const QString& app ) const;

    static void setStyle( int style );

public slots:
    void slotKill( bool force = false );

signals:
    void sigRequestNewNote();
    void sigShowNextNote();
    void sigNameChanged();
    void sigDataChanged();
    void sigColorChanged();
    void sigKillNote( KCal::Journal* );

    void sigFindFinished();

protected:
    virtual void drawFrame( QPainter* );
    virtual void showEvent( QShowEvent* );
    virtual void resizeEvent( QResizeEvent* );
    virtual void closeEvent( QCloseEvent* );
    virtual void dropEvent( QDropEvent* );
    virtual void dragEnterEvent( QDragEnterEvent* );

    virtual bool event( QEvent* );
    virtual bool eventFilter( QObject*, QEvent* );

    virtual bool focusNextPrevChild( bool );

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
    void slotHighlight( const QString& txt, int idx, int len );

    void slotApplyConfig();
    void slotUpdateKeepAboveBelow();
    void slotUpdateShowInTaskbar();
    void slotUpdateDesktopActions();

    void slotUpdateViewport( int, int );

private:
    void updateFocus();
    void updateMask();
    void updateLayout();
    void updateLabelAlignment();
    void updateBackground( int offset = -1 );

    void createFold();

    void toDesktop( int desktop );

    QString toPlainText( const QString& );

private:
    QLabel        *m_label, *m_pushpin, *m_fold;
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
};

#endif
