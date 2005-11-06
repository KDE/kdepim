/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCLEANUP_H
#define KNCLEANUP_H

#include <QList>

class Q3ProgressBar;
class QCloseEvent;
class QLabel;

class KNArticleCollection;
class KNGroup;
class KNFolder;

namespace KNode {
class Cleanup;
}


/** This class handles group expiration and folder compaction. */
class KNCleanUp {

  public:
    KNCleanUp();
    ~KNCleanUp();

    void appendCollection(KNArticleCollection *c)   { mColList.append( c ); }
    void start();
    void reset();

    void expireGroup( KNGroup *g, bool showResult = false );
    /** Compacts the given folder, ie. remove all deleted messages from the
     * mbox file.
     * @param f The folder to compact.
     */
    void compactFolder(KNFolder *f);

  protected:

    /** Cleanup progress dialog. */
    class ProgressDialog : public QDialog  {

      public:
        ProgressDialog(int steps);
        ~ProgressDialog();

        void showMessage(const QString &s);
        void doProgress();

      protected:
        void closeEvent(QCloseEvent *e);

        QLabel *m_sg;
        Q3ProgressBar *p_bar;

        int s_teps, p_rogress;
    };

    ProgressDialog *d_lg;
    QList<KNArticleCollection*> mColList;

};

#endif
