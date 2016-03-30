/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/
#ifndef BLOGILOCOMPOSERWebEngineEDITOR_H
#define BLOGILOCOMPOSERWebEngineEDITOR_H

#include "blogilocomposerwebengineview.h"

#include <ComposerEditorNG/ComposerEditor>

class BilboMedia;
class QAction;
class BlogiloComposerWebEngineEditor : public ComposerEditorNG::ComposerEditor
{
    Q_OBJECT
public:
    explicit BlogiloComposerWebEngineEditor(BlogiloComposerWebEngineView *view, QWidget *parent);
    ~BlogiloComposerWebEngineEditor();

    void setReadOnly(bool _readOnly);
    QList< BilboMedia * > getLocalImages();
    void replaceImageSrc(const QString &src, const QString &dest);

    void startEditing();

    void insertShortUrl(const QString &url);

private Q_SLOTS:
    void slotAddPostSplitter();
    void slotToggleCode(bool);

private:
    void execCommand(const QString &cmd, const QString &arg);
    void execCommand(const QString &cmd);
    bool readOnly;
    QAction *mActSplitPost;
    QAction *mActCode;
};

#endif // BLOGILOCOMPOSERWebEngineEDITOR_H
