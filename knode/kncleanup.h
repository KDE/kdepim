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

#include <tqsemimodal.h>

class QProgressBar;
class QLabel;

class KNArticleCollection;
class KNGroup;
class KNFolder;

namespace KNConfig {
class Cleanup;
}


class KNCleanUp {

  public:
    KNCleanUp();
    ~KNCleanUp();

    void appendCollection(KNArticleCollection *c)   { mColList.append( c ); }
    void start();
    void reset();

    void expireGroup( KNGroup *g, bool showResult = false );
    void compactFolder(KNFolder *f);

  protected:

    class ProgressDialog : public TQDialog  {

      public:
        ProgressDialog(int steps);
        ~ProgressDialog();

        void showMessage(const TQString &s);
        void doProgress();

      protected:
        void closeEvent(TQCloseEvent *e);

        TQLabel *m_sg;
        TQProgressBar *p_bar;

        int s_teps, p_rogress;
    };

    ProgressDialog *d_lg;
    TQValueList<KNArticleCollection*> mColList;

};

#endif

// kate: space-indent on; indent-width 2;
