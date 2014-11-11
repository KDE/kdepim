/* Copyright (C) 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SIEVEEDITORTEXTMODEWIDGET_H
#define SIEVEEDITORTEXTMODEWIDGET_H

#include "sieveeditorabstractwidget.h"

class QSplitter;
class KTabWidget;

namespace PimCommon {
class PlainTextEditFindBar;
class PlainTextEditor;
class TextGoToLineWidget;
class SlideContainer;
}

namespace KSieveUi {
class SieveInfoWidget;
class SieveTextEdit;
class SieveEditorWarning;
class SieveEditorParsingMissingFeatureWarning;
class SieveEditorTabWidget;
class SieveEditorTextModeWidget : public SieveEditorAbstractWidget
{
    Q_OBJECT
public:
    explicit SieveEditorTextModeWidget(QWidget *parent=0);
    ~SieveEditorTextModeWidget();


    void setSieveCapabilities( const QStringList &capabilities );

    QString script() const;
    void setScript( const QString &script );

    void setDebugScript( const QString &debug );

    QString currentscript();
    void setImportScript( const QString &script );

    void autoGenerateScripts();
    void generateXml();

    void showEditorWarning();
    void hideEditorWarning();

    void showParsingEditorWarning();

    void setParsingEditorWarningError(const QString &script, const QString &error);
    void goToLine();
Q_SIGNALS:
    void enableButtonOk( bool );
    void switchToGraphicalMode();
    void valueChanged();

private slots:
    void slotTextChanged();
    void slotFind();
    void slotReplace();    
    void slotGoToLine(int line);
    void slotShowGoToLine();
private:
    void readConfig();
    void writeConfig();
    bool loadFromFile( const QString &filename );
    QString mOriginalScript;
    QStringList mSieveCapabilities;
    SieveTextEdit * mTextEdit;
    PimCommon::PlainTextEditor *mDebugTextEdit;
    PimCommon::PlainTextEditFindBar *mFindBar;
    SieveInfoWidget *mSieveInfo;
    QSplitter *mMainSplitter;
    QSplitter *mExtraSplitter;
    QSplitter *mTemplateSplitter;
    SieveEditorWarning *mSieveEditorWarning;
    SieveEditorParsingMissingFeatureWarning *mSieveParsingWarning;
    SieveEditorTabWidget *mTabWidget;
    PimCommon::TextGoToLineWidget *mGoToLine;
    PimCommon::SlideContainer *mSliderContainer;
};

}

#endif // SIEVEEDITORTEXTMODEWIDGET_H
