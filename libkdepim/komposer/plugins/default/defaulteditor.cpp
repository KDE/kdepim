/**
 * defaulteditor.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "defaulteditor.h"
#include "core.h"

#include <kgenericfactory.h>
#include <kapplication.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <kaction.h>
#include <kcolordialog.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kprinter.h>
#include <kfinddialog.h>
#include <kfind.h>
#include <kreplacedialog.h>
#include <kreplace.h>

#include <tqtextedit.h>
#include <tqwidget.h>

typedef KGenericFactory<DefaultEditor> DefaultEditorFactory;
K_EXPORT_COMPONENT_FACTORY( libkomposer_defaulteditor,
                            DefaultEditorFactory( "komposer_defaulteditor" ) )

DefaultEditor::DefaultEditor( TQObject *parent, const char *name, const TQStringList &args )
  : Editor( parent, name, args ), m_textEdit( 0 )
{
  setInstance( DefaultEditorFactory::instance() );

  m_textEdit = new TQTextEdit( 0 );

  createActions( actionCollection() );

  setXMLFile( "defaulteditorui.rc" );
}

DefaultEditor::~DefaultEditor()
{
}


TQWidget*
DefaultEditor::widget()
{
    return m_textEdit;
}

QString
DefaultEditor::text() const
{
  return m_textEdit->text();
}

void
DefaultEditor::setText( const TQString &text )
{
  m_textEdit->setText( text );
}

void
DefaultEditor::changeSignature( const TQString &sig )
{
  TQString text = m_textEdit->text();

  int sigStart = text.findRev( "-- " );
  TQString sigText = TQString( "-- \n%1" ).arg( sig );

  text.replace( sigStart, text.length(), sigText );
}

void
DefaultEditor::createActions( KActionCollection *ac )
{
  //
  // File Actions
  //
  (void) KStdAction::open( this, TQT_SLOT(open()), ac );
  (void) KStdAction::openRecent( this, TQT_SLOT(openURL(const KURL &)), ac );
  (void) KStdAction::save( this, TQT_SLOT(save()), ac );
  (void) KStdAction::saveAs( this, TQT_SLOT(saveAs()), ac );

  //
  // Edit Actions
  //
  KAction *actionUndo = KStdAction::undo( m_textEdit, TQT_SLOT(undo()), ac );
  actionUndo->setEnabled( false );
  connect( m_textEdit, TQT_SIGNAL(undoAvailable(bool)),
           actionUndo, TQT_SLOT(setEnabled(bool)) );

  KAction *actionRedo = KStdAction::redo( m_textEdit, TQT_SLOT(redo()), ac );
  actionRedo->setEnabled( false );
  connect( m_textEdit, TQT_SIGNAL(redoAvailable(bool)),
           actionRedo, TQT_SLOT(setEnabled(bool)) );

  KAction *action_cut = KStdAction::cut( m_textEdit, TQT_SLOT(cut()), ac );
  action_cut->setEnabled( false );
  connect( m_textEdit, TQT_SIGNAL(copyAvailable(bool)),
           action_cut, TQT_SLOT(setEnabled(bool)) );

  KAction *action_copy = KStdAction::copy( m_textEdit, TQT_SLOT(copy()), ac );
  action_copy->setEnabled( false );
  connect( m_textEdit, TQT_SIGNAL(copyAvailable(bool)),
           action_copy, TQT_SLOT(setEnabled(bool)) );

  (void) KStdAction::print( this, TQT_SLOT(print()), ac );

  (void) KStdAction::paste( m_textEdit, TQT_SLOT(paste()), ac );
  (void) new KAction( i18n( "C&lear" ), 0,
                      m_textEdit, TQT_SLOT(removeSelectedText()),
                      ac, "edit_clear" );

  (void) KStdAction::selectAll( m_textEdit, TQT_SLOT(selectAll()), ac );

  //
  // View Actions
  //
  (void) KStdAction::zoomIn( m_textEdit, TQT_SLOT(zoomIn()), ac );
  (void) KStdAction::zoomOut( m_textEdit, TQT_SLOT(zoomOut()), ac );

  //
  // Character Formatting
  //
  m_actionBold = new KToggleAction( i18n("&Bold"), "text_bold", CTRL+Key_B,
                                    ac, "format_bold" );
  connect( m_actionBold, TQT_SIGNAL(toggled(bool)),
           m_textEdit, TQT_SLOT(setBold(bool)) );

  m_actionItalic = new KToggleAction( i18n("&Italic"), "text_italic", CTRL+Key_I,
                                      ac, "format_italic" );

  connect( m_actionItalic, TQT_SIGNAL(toggled(bool)),
           m_textEdit, TQT_SLOT(setItalic(bool) ));

  m_actionUnderline = new KToggleAction( i18n("&Underline"), "text_under", CTRL+Key_U,
                                         ac, "format_underline" );

  connect( m_actionUnderline, TQT_SIGNAL(toggled(bool)),
           m_textEdit, TQT_SLOT(setUnderline(bool)) );

  (void) new KAction( i18n("Text &Color..."), "colorpicker", 0,
                      this, TQT_SLOT(formatColor()),
                      ac, "format_color" );

  //
  // Font
  //
  m_actionFont = new KFontAction( i18n("&Font"), 0,
                                 ac, "format_font" );
  connect( m_actionFont, TQT_SIGNAL(activated(const TQString &)),
           m_textEdit, TQT_SLOT(setFamily(const TQString &)) );


  m_actionFontSize = new KFontSizeAction( i18n("Font &Size"), 0,
                                          ac, "format_font_size" );
  connect( m_actionFontSize, TQT_SIGNAL(fontSizeChanged(int)),
           m_textEdit, TQT_SLOT(setPointSize(int)) );

  //
  // Alignment
  //
  m_actionAlignLeft = new KToggleAction( i18n("Align &Left"), "text_left", 0,
                                         ac, "format_align_left" );
  connect( m_actionAlignLeft, TQT_SIGNAL(toggled(bool)),
           this, TQT_SLOT(setAlignLeft(bool)) );

  m_actionAlignCenter = new KToggleAction( i18n("Align &Center"), "text_center", 0,
                                           ac, "format_align_center" );
  connect( m_actionAlignCenter, TQT_SIGNAL(toggled(bool)),
           this, TQT_SLOT(setAlignCenter(bool)) );

  m_actionAlignRight = new KToggleAction( i18n("Align &Right"), "text_right", 0,
                                          ac, "format_align_right" );
  connect( m_actionAlignRight, TQT_SIGNAL(toggled(bool)),
           this, TQT_SLOT(setAlignRight(bool)) );

  m_actionAlignJustify = new KToggleAction( i18n("&Justify"), "text_block", 0,
                                            ac, "format_align_justify" );
  connect( m_actionAlignJustify, TQT_SIGNAL(toggled(bool)),
           this, TQT_SLOT(setAlignJustify(bool)) );

  m_actionAlignLeft->setExclusiveGroup( "alignment" );
  m_actionAlignCenter->setExclusiveGroup( "alignment" );
  m_actionAlignRight->setExclusiveGroup( "alignment" );
  m_actionAlignJustify->setExclusiveGroup( "alignment" );

  //
  // Tools
  //
  (void) KStdAction::spelling( this, TQT_SLOT(checkSpelling()), ac );

  //
  // Setup enable/disable
  //
  updateActions();

  connect( m_textEdit, TQT_SIGNAL(currentFontChanged(const TQFont &)),
           this, TQT_SLOT( updateFont() ) );
  connect( m_textEdit, TQT_SIGNAL(currentFontChanged(const TQFont &)),
           this, TQT_SLOT(updateCharFmt()) );
  connect( m_textEdit, TQT_SIGNAL(cursorPositionChanged(int, int)),
           this, TQT_SLOT(updateAligment()) );
}

void
DefaultEditor::updateActions()
{
  updateCharFmt();
  updateAligment();
  updateFont();
}

void
DefaultEditor::updateCharFmt()
{
  m_actionBold->setChecked( m_textEdit->bold() );
  m_actionItalic->setChecked( m_textEdit->italic() );
  m_actionUnderline->setChecked( m_textEdit->underline() );
}

void
DefaultEditor::updateAligment()
{
  int align = m_textEdit->alignment();

  switch ( align ) {
  case AlignRight:
    m_actionAlignRight->setChecked( true );
    break;
  case AlignCenter:
    m_actionAlignCenter->setChecked( true );
    break;
  case AlignLeft:
    m_actionAlignLeft->setChecked( true );
    break;
  case AlignJustify:
    m_actionAlignJustify->setChecked( true );
    break;
  default:
    break;
  }
}

void
DefaultEditor::updateFont()
{
  if ( m_textEdit->pointSize() > 0 )
    m_actionFontSize->setFontSize( m_textEdit->pointSize() );
  m_actionFont->setFont( m_textEdit->family() );
}

void
DefaultEditor::formatColor()
{
  TQColor col;

  int s = KColorDialog::getColor( col, m_textEdit->color(), m_textEdit );
  if ( s != TQDialog::Accepted )
    return;

  m_textEdit->setColor( col );
}

void
DefaultEditor::setAlignLeft( bool yes )
{
  if ( yes )
    m_textEdit->setAlignment( AlignLeft );
}

void
DefaultEditor::setAlignRight( bool yes )
{
  if ( yes )
    m_textEdit->setAlignment( AlignRight );
}

void
DefaultEditor::setAlignCenter( bool yes )
{
  if ( yes )
    m_textEdit->setAlignment( AlignCenter );
}

void
DefaultEditor::setAlignJustify( bool yes )
{
  if ( yes )
    m_textEdit->setAlignment( AlignJustify );
}

//
// Content Actions
//

bool
DefaultEditor::open()
{
  KURL url = KFileDialog::getOpenURL();
  if ( url.isEmpty() )
    return false;

  //fixme
  //return openURL( url );
  return true;
}

bool
DefaultEditor::saveAs()
{
  KURL url = KFileDialog::getSaveURL();
  if ( url.isEmpty() )
    return false;

  //FIXME
  //return KParts::ReadWritePart::saveAs( url );
  return true;
}

void
DefaultEditor::checkSpelling()
{
  TQString s;
  if ( m_textEdit->hasSelectedText() )
    s = m_textEdit->selectedText();
  else
    s = m_textEdit->text();

  //KSpell::modalCheck( s );
}

bool
DefaultEditor::print()
{
  return true;
}

#include "defaulteditor.moc"
