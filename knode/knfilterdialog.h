/*
    knfilterdialog.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNFILTERDIALOG_H
#define KNFILTERDIALOG_H

#include <kdialogbase.h>

class KNFilterConfigWidget;
class KNArticleFilter;
class KLineEdit;
class QComboBox;
class QCheckBox;


class KNFilterDialog : public KDialogBase {

  Q_OBJECT

  friend class KNFilterManager;

  public:
    KNFilterDialog(KNArticleFilter *f=0, QWidget *parent=0, const char *name=0);
    ~KNFilterDialog();

    KNArticleFilter* filter() { return fltr; }

  protected:
    KNFilterConfigWidget *fw;
    KLineEdit *fname;
    QComboBox *apon;
    QCheckBox *enabled;

    KNArticleFilter *fltr;

  protected slots:
    void slotOk();
    void slotTextChanged( const QString & );
};

#endif
