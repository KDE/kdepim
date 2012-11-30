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

class BilboMedia;
class QWebView;
class QAction;
class TextEditor;
class KSelectAction;
class KAction;
class KToolBar;
class KToggleAction;
class WebView : public KWebView {
    Q_OBJECT

public:
    WebView ( QWidget* = 0 );
    //just shows the cursor.
    void startEditing();

protected slots:
    void sendMouseReleaseEvent();

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event);
//     virtual void keyPressEvent ( QKeyEvent* );
    virtual void dragEnterEvent ( QDragEnterEvent* );
    virtual void dropEvent ( QDropEvent* );
};

//----------------------------------------------------------------------

class TextEditor : public QWidget {
    Q_OBJECT

public:
    TextEditor ( QWidget* = 0 );

    enum WebAction { AlignLeft = 100, AlignRight, AlignCenter, AlignJustify, StrikeThrough, NumberedList, BulletedList};

    void setReadOnly ( bool );

    QAction* getAction ( QWebPage::WebAction action ) const;

//     void addCommand ( QUndoCommand* );

    void replaceImageSrc(const QString& src, const QString& dest);

    QString plainTextContent();
    QString htmlContent();

    QList<BilboMedia*> getLocalImages();

public slots:
    void clear();
    //just shows the cursor.
    void startEditing();
    void setHtmlContent ( const QString &arg1 );

    void formatIncreaseIndent();
    void formatDecreaseIndent();
    void slotToggleCode(bool);
    void slotChangeFormatType(const QString& formatText);
    void slotIncreaseFontSize();
    void slotDecreaseFontSize();
    void formatTextColor();
    void slotToggleBlockQuote(bool);
    void slotAddLink();
    void slotRemoveLink();
    void slotAddImage();
    void slotChangeLayoutDirection(bool rightToLeft);
    void slotAddPostSplitter();
    void slotToggleSpellChecking(bool);

signals:
    void editingFinished();
    void editingFinishKeyPressed();
    void selectionChanged();
    void contentsChanged();
    void sigShowStatusMessage ( const QString&, bool );
    void sigBusy ( bool );
    void textChanged();

protected:
    virtual void focusInEvent ( QFocusEvent* );

    //the range of font size is between 0 and 6
    // (xx-small to xx-large)
    void setFontSize ( int );
    int getFontSize() const;

    Qt::Alignment getAlignment() const;

    //This is only a temporary zoom factor which will be
    //changed back to Settings::settings() -> cardAppearanceZoomFactor
    //whenever you use setDocument();
//     void setZoomFactor ( double );
    double getZoomFactor() const;


private slots:
    void adjustActions();
    void somethingEdittedSlot();
    void updateStyle();
    void dropMimeDataSlot ( const QMimeData* );
    void anObjectIsModifiedSlot();

private:
//       mutable Document document;
    WebView *webView;
    QList<int> fontSizes;
    bool readOnly;
    bool objectsAreModified;
    KToolBar *barVisual;

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
    KToggleAction *actOrderedList;
    KToggleAction *actUnorderedList;
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

};

#endif

