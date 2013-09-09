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

class KTextEdit;
class QPushButton;
class QSplitter;

namespace PimCommon {
class PlainTextEditFindBar;
}

namespace KSieveUi {
class SieveInfoWidget;
class SieveTextEdit;
class SieveEditorWarning;
class SieveEditorParsingWarning;
class SieveEditorTextModeWidget : public SieveEditorAbstractWidget
{
    Q_OBJECT
public:
    explicit SieveEditorTextModeWidget(QWidget *parent=0);
    ~SieveEditorTextModeWidget();


    void setSieveCapabilities( const QStringList &capabilities );

    QString script() const;
    void setScript( const QString &script );

    void setDebugColor( const QColor &col );
    void setDebugScript( const QString &debug );

    QString currentscript();
    void setImportScript( const QString &script );

    void autoGenerateScripts();
    void generateXml();

    void showEditorWarning();
    void hideEditorWarning();

    void showParsingEditorWarning();

    void setParsingEditorWarningError(const QString &script, const QString &error);

Q_SIGNALS:
    void enableButtonOk( bool );
    void switchToGraphicalMode();

private slots:
    void slotTextChanged();
    void slotFind();

private:
    void readConfig();
    void writeConfig();
    bool loadFromFile( const QString &filename );
    QString mOriginalScript;
    QStringList mSieveCapabilities;
    SieveTextEdit * mTextEdit;
    KTextEdit *mDebugTextEdit;
    PimCommon::PlainTextEditFindBar *mFindBar;
    SieveInfoWidget *mSieveInfo;
    QSplitter *mMainSplitter;
    QSplitter *mExtraSplitter;
    QSplitter *mTemplateSplitter;
    SieveEditorWarning *mSieveEditorWarning;
    SieveEditorParsingWarning *mSieveParsingWarning;
};

}

#endif // SIEVEEDITORTEXTMODEWIDGET_H
