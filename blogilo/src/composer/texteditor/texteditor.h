/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "texteditor.h" from
    FlashQard project.

    Copyright (C) 2008-2009 Shahab <shahab@flashqard-project.org>
    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/


#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QTextEdit>
#include <QLineEdit>
#include <KWebView>
#include <QUndoCommand>
#include <qjson/parser.h>

class BilboMedia;
class QWebView;
class QAction;
class TextEditor;
class KSelectAction;
class KAction;
class KToolBar;

class ObjectsForJavaScript : public QObject {
    Q_OBJECT

public:
    ObjectsForJavaScript ( TextEditor* );

signals:
    void clickedOnObjectSignal ( const QString& );
    void clickedOnNonObjectSignal();

public slots:
    QString getCss();
    QString getARandomName();
    QString getObjectPath ( const QString& );
    void clickedOnObject ( const QString& );
    void clickedOnNonObject();

private:
    TextEditor *textEditor;
};

//----------------------------------------------------------------------

class WebView : public KWebView {
    Q_OBJECT

public:
    WebView ( QWidget* = 0 );
    //just shows the cursor.
    void startEditing();

signals:
    void focusOutSignal();
    void editingFinishKeyPressed();
    void dropMimeDataSignal ( const QMimeData* );

private slots:
    void sendMouseReleaseEvent();

//   private slots:
//      void emulateMouseRelease();

protected:
    virtual void focusOutEvent ( QFocusEvent* );
    virtual void focusInEvent ( QFocusEvent* );
    virtual void keyPressEvent ( QKeyEvent* );
    virtual void dragEnterEvent ( QDragEnterEvent* );
    virtual void dropEvent ( QDropEvent* );
};

//----------------------------------------------------------------------

class TextEditorObject : public QObject {
    Q_OBJECT

public:
    TextEditorObject ( TextEditor*, const QString& );
    virtual ~TextEditorObject() {}

    void setId ( const QString& );
    QString getId() const;

    virtual QString getType() const = 0;
    virtual QByteArray getByteArray() const = 0;
    //if replace is true, id gets changed
    virtual void setByteArray ( const QByteArray&, bool replace = false ) = 0;

signals:
    void modifiedSignal();

protected:
    TextEditor *textEditor;

private:
    QString id;
};

class TextEditorObjectImage : public TextEditorObject {
    Q_OBJECT

public:
    TextEditorObjectImage ( TextEditor*, const QString&, const QImage& = QImage() );
    TextEditorObjectImage ( TextEditor*, const QString&, const QByteArray& = QByteArray() );
    virtual ~TextEditorObjectImage();

    void setImage ( const QImage&, bool = false );
    QImage getImage() const;

    virtual QString getType() const;
    virtual QByteArray getByteArray() const;
    virtual void setByteArray ( const QByteArray&, bool = false );

    QString getFilePath() const;

private:
    QString filePath;

    void removeFile ( const QString& );
};

//----------------------------------------------------------------------

class TextEditor : public QWidget {
    Q_OBJECT

public:
    TextEditor ( QWidget* = 0 );

    enum WebAction { AlignLeft = 100, AlignRight, AlignCenter, AlignJustify, StrikeThrough, NumberedList, BulletedList};

    void setReadOnly ( bool );

    void reload();

    bool insertImage ( const QString& );
    bool insertImage ( const QByteArray& );

//     void setFontFamily ( const QString& );
//     QString getFontFamily() const;

    //the range of font size is between 0 and 6
    // (xx-small to xx-large)
    void setFontSize ( int );
    int getFontSize() const;
/*
    void setForegroundColor ( const QColor& );
    QColor getForegroundColor() const;

    void setBackgroundColor ( const QColor& );
    QColor getBackgroundColor() const;*/

//     void setAlignment ( Qt::Alignment );
    Qt::Alignment getAlignment() const;

    //This is only a temporary zoom factor which will be
    //changed back to Settings::settings() -> cardAppearanceZoomFactor
    //whenever you use setDocument();
    void setZoomFactor ( double );
    double getZoomFactor() const;

    QAction* getAction ( QWebPage::WebAction action ) const;

    void addCommand ( QUndoCommand* );

    void replaceImageSrc(const QString& src, const QString& dest);

public slots:
    QList<BilboMedia*> getLocalImages();
//       void finishEditing(); //emits editingFinished signal if anything have been edited
    void clear();
    //just shows the cursor.
    void startEditing();
    void setCurrentTitle ( const QString& title );
    void setHtmlContent ( const QString &arg1 );
    QString htmlContent();
    bool updateMediaPaths();

//     void styleParagraph();
//     void styleHeading1();
//     void styleHeading2();
//     void styleHeading3();
//     void styleHeading4();
//     void styleHeading5();
//     void styleHeading6();
//     void stylePreformatted();
//     void styleAddress();
//     void formatStrikeThrough();
//     void formatAlignLeft();
//     void formatAlignCenter();
//     void formatAlignRight();
//     void formatAlignJustify();
    void formatIncreaseIndent();
    void formatDecreaseIndent();
//     void formatNumberedList();
//     void formatBulletedList();

    void slotToggleCode(bool);
    void slotChangeFormatType(const QString& formatText);
    void slotIncreaseFontSize();
    void slotDecreaseFontSize();
    void formatTextColor();
    void slotToggleBlockQuote(bool);
    void slotAddEditLink();
    void slotRemoveLink();
    void slotAddImage();
    void slotChangeLayoutDirection(bool rightToLeft);
    void slotAddPostSplitter();
    void slotToggleSpellChecking(bool);
    /*
    void alignLeftSlot();
    void alignCenterSlot();
    void alignRightSlot();
    void alignJustifySlot();
    void strikeThroughSlot();
    void numberedListSlot();
    void bulletedListSlot();

    void alignLeftSlot();
    void alignRightSlot();
    void alignCenterSlot();
    void alignJustifySlot();
    void strikeThroughSlot();
    void numberedListSlot();
    void bulletedListSlot();*/

signals:
    void editingFinished();
    void editingFinishKeyPressed();
    void selectionChanged();
    void contentsChanged();
    void clickedOnObjectSignal ( TextEditorObject* );
    void clickedOnNonObjectSignal();
    void sigShowStatusMessage ( const QString&, bool );
    void sigBusy ( bool );

protected:
    virtual void focusInEvent ( QFocusEvent* );

private slots:
    void adjustActions();
    void addJavaScriptObjectSlot();
    void somethingEdittedSlot();
    void updateStyle();
    void dropMimeDataSlot ( const QMimeData* );
    void clickedOnObjectSlot ( const QString& );
    void clickedOnNonObjectSlot();
    void anObjectIsModifiedSlot();

private:
//       mutable Document document;
    WebView *webView;
    ObjectsForJavaScript *objForJavaScript;
    QList<int> fontSizes;
    //first argument is object's id, and the other one is object's
    //file path
    QList<TextEditorObject*> objects;
    bool readOnly;
    bool objectsAreModified;
    QJson::Parser parser;
    KToolBar *barVisual;
//     KAction *alignLeftAction;
//     KAction *alignRightAction;
//     KAction *alignCenterAction;
//     KAction *alignJustifyAction;
//     KAction *strikeThroughAction;
//     KAction *numberedListAction;
//     KAction *bulletedListAction;

    KAction *actBold;
    KAction *actItalic;
    KAction *actUnderline;
    KAction *actStrikeout;
    KAction *actCode;
    KAction *actFontIncrease;
    KAction *actFontDecrease;
    KAction *actNewParagraph;
    KAction *actAlignRight;
    KAction *actAlignLeft;
    KAction *actAlignCenter;
    KAction *actJustify;
    KAction *actRightToLeft;
    KAction *actAddLink;
    KAction *actRemoveLink;
    KAction *actRemoveFormatting;
    KAction *actColorSelect;
    KAction *actAddImage;
//     KAction *actAddMedia;
    KAction *actOrderedList;
    KAction *actUnorderedList;
    KAction *actBlockQuote;
    KAction *actSplitPost;
    KAction *actCheckSpelling;
    KSelectAction *actFormatType;

    void createLayout();
    void createActions();
    void execCommand ( const QString&, const QString& );
    void execCommand ( const QString &cmd );
    bool queryCommandState ( const QString &cmd );
    QVariant evaluateJavaScript ( const QString&, bool );
    void adjustFontSizes();
    //gets a string like "rgb(0, 0, 0)" and returns a QColor
    QColor rgbToColor ( QString ) const;
    QString getHtml() const;

    QString addObject ( TextEditorObject* );
//      QString getObjectPath(const QString&) const;
    void clearObjects();
    void addDocumentAttachmentsToObjects();
    QByteArray loadImage ( const QString& );
//      void removeUnusedAttachments();
    TextEditorObject* findObject ( const QString& );
    void addUsedAttachmentsToDocument();

//       friend class EmbeddedTextEditor;
    friend class ObjectsForJavaScript;
    friend class CommandResizeImage;
    friend class TextEditorObjectImage;
};

//----------------------------------------------------------------------

class CommandResizeImage : public QUndoCommand {
public:
    CommandResizeImage ( TextEditorObjectImage*, const QSize& );
    virtual void undo();
    virtual void redo();

private:
    TextEditorObjectImage *imageObject;
    QImage initialImage;
    QSize newSize;
};

class CommandFlipImage : public QUndoCommand {
public:
    CommandFlipImage ( TextEditorObjectImage*, Qt::Orientation );
    virtual void undo();
    virtual void redo();

private:
    TextEditorObjectImage *imageObject;
    Qt::Orientation orientation;
};

class CommandRotateImage : public QUndoCommand {
public:
    enum Rotation {RotateRight, RotateLeft};
    CommandRotateImage ( TextEditorObjectImage*, Rotation );
    virtual void undo();
    virtual void redo();

private:
    TextEditorObjectImage *imageObject;
    Rotation rotation;
};

#endif

