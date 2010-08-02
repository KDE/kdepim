/* -*- mode: C++; c-file-style: "gnu" -*-
 * KMComposeWin Header File
 * Author: Markus Wuebben <markus.wuebben@kde.org>
 */
#ifndef __KMAIL_KMEDIT_H__
#define __KMAIL_KMEDIT_H__

#include <kdeversion.h>
#include <keditcl.h>
#include <tqmap.h>
#include <tqstringlist.h>
#include <tqclipboard.h>

class KMComposeWin;
class KSpellConfig;
class KSpell;
class SpellingFilter;
class KTempFile;
class KDictSpellingHighlighter;
class KDirWatch;
class KProcess;
class TQPopupMenu;


class KMEdit : public KEdit {
  Q_OBJECT
public:
  KMEdit(TQWidget *parent=0,KMComposeWin* composer=0,
         KSpellConfig* spellConfig = 0,
	 const char *name=0);
  ~KMEdit();

  /**
   * Start the spell checker.
   */
  void spellcheck();

  /**
   * Text with lines breaks inserted after every row
   */
  TQString brokenText();

   /**
   * Toggle automatic spellchecking
   */
  int autoSpellChecking( bool );

  /**
   * For the external editor
   */
  void setUseExternalEditor( bool use ) { mUseExtEditor = use; }
  void setExternalEditorPath( const TQString & path ) { mExtEditor = path; }

  /**
   * Check that the external editor has finished and output a warning
   * if it hasn't.
   * @return false if the user chose to cancel whatever operation
   * called this method.
   */
  bool checkExternalEditorFinished();

  TQPopupMenu* createPopupMenu(const TQPoint&);
  void setSpellCheckingActive(bool spellCheckingActive);

  /** Drag and drop methods */
  void contentsDragEnterEvent(TQDragEnterEvent *e);
  void contentsDragMoveEvent(TQDragMoveEvent *e);
  void contentsDropEvent(TQDropEvent *e);

  void deleteAutoSpellChecking();

  unsigned int lineBreakColumn() const;
  
  /** set cursor to absolute position pos */
  void setCursorPositionFromStart(unsigned int pos);

signals:
  void spellcheck_done(int result);
  void attachPNGImageData(const TQByteArray &image);
  void pasteImage();
  void focusUp();
  void focusChanged( bool );
  void insertSnippet();
public slots:
  void initializeAutoSpellChecking();
  void slotSpellcheck2(KSpell*);
  void slotSpellResult(const TQString&);
  void slotSpellDone();
  void slotExternalEditorDone(KProcess*);
  void slotMisspelling(const TQString &, const TQStringList &, unsigned int);
  void slotCorrected (const TQString &, const TQString &, unsigned int);
  void addSuggestion(const TQString& text, const TQStringList& lst, unsigned int );
  void cut();
  void clear();
  void del();
  void paste();
protected:
  /**
   * Event filter that does Tab-key handling.
   */
  bool eventFilter(TQObject*, TQEvent*);
  void keyPressEvent( TQKeyEvent* );
  
  void contentsMouseReleaseEvent( TQMouseEvent * e );

private slots:
  void slotExternalEditorTempFileChanged( const TQString & fileName );

private:
  void killExternalEditor();

private:
  KMComposeWin* mComposer;

  KSpell *mKSpell;
  KSpellConfig *mSpellConfig;
  TQMap<TQString,TQStringList> mReplacements;
  SpellingFilter* mSpellingFilter;
  KTempFile *mExtEditorTempFile;
  KDirWatch *mExtEditorTempFileWatcher;
  KProcess  *mExtEditorProcess;
  bool      mUseExtEditor;
  TQString   mExtEditor;
  bool      mWasModifiedBeforeSpellCheck;
  KDictSpellingHighlighter *mSpellChecker;
  bool mSpellLineEdit;
  QClipboard::Mode mPasteMode;
};

#endif // __KMAIL_KMEDIT_H__

