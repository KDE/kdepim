/*
  Copyright (c) 2012-2013 Montel Laurent <montel.org>

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

#ifndef COMPOSEREDITOR_H
#define COMPOSEREDITOR_H

#include "composereditor_export.h"
#include "composerview.h"

#include <QWidget>
#include <QWebPage>

class KActionCollection;
class KAction;

namespace ComposerEditorNG
{
class ComposerEditorPrivate;
class ComposerView;
class COMPOSEREDITORNG_EXPORT ComposerEditor : public QWidget
{
    Q_OBJECT
public:
    Q_PROPERTY(bool enableRichText READ enableRichText WRITE setEnableRichText)

    explicit ComposerEditor(QWidget *parent);
    explicit ComposerEditor(ComposerView *view, QWidget *parent);

    ~ComposerEditor();

    virtual void addCreatedActionsToActionCollection(KActionCollection *actionCollection);

    /**
     * @brief createActions create a list of action from default action.
     */
    void createActions(const QList<ComposerView::ComposerViewAction>&);

    /**
     * @brief createAllActions. Create all actions.
     */
    void createAllActions();

    /**
     * @brief createToolBar.
     */
    void createToolBar(const QList<ComposerView::ComposerViewAction>&);

    /**
     * @brief addActionInToolBar add action in toolbar. QAction that is not in default action from composerview.
     * @param act
     */
    void addActionInToolBar(QAction *act);

    /**
     * @brief toolbar
     * @return a pointer from toolbar.
     */
    KToolBar *toolbar() const;

    /**
     * @brief plainTextContent
     * @return text as plain text.
     */
    QString plainTextContent() const;

    /**
     * @brief htmlContent
     * @return the html content.
     */
    QString htmlContent() const;
    /**
     * @brief setHtmlContent
     * @param html set html in composerview.
     */
    void setHtmlContent( const QString& html );


    bool enableRichText() const;

    /**
     * @brief isModified
     * @return true if document is modify.
     */
    bool isModified() const;

    /**
     * @brief action
     * @param action is the enim from QWebPage to define specific action
     * @return a QAction from QWebPage
     */
    QAction* action(QWebPage::WebAction action);

    ComposerView *view() const;

public Q_SLOTS:
    void setEnableRichText(bool richTextEnabled);
    void paste();
    void cut();
    void copy();
    void undo();
    void redo();

Q_SIGNALS:
    void openLink(const QUrl&);
    void textChanged();

private:
    friend class ComposerEditorPrivate;
    ComposerEditorPrivate * const d;
};
}

#endif // COMPOSEREDITOR_H
