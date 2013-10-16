/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TEMPLATESTEXTEDITOR_H
#define TEMPLATESTEXTEDITOR_H
#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"
class QCompleter;
class QKeyEvent;
namespace TemplateParser {
class TemplatesTextEditor : public PimCommon::RichTextEditor
{
    Q_OBJECT
public:
    explicit TemplatesTextEditor(QWidget *parent=0);
    ~TemplatesTextEditor();

protected Q_SLOTS:
    void slotInsertCompletion( const QString & );
    QString wordUnderCursor();

protected:
    void initCompleter();
    void keyPressEvent( QKeyEvent *e );

private:
    QCompleter *m_completer;
};
}
#endif // TEMPLATESTEXTEDITOR_H
