/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/adduseriddialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "adduseriddialog.h"

#include "ui_adduseriddialog.h"

#include <utils/validation.h>

#include <kleo/stl_util.h>

#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QValidator>

#include <KConfigGroup>
#include <KLocalizedString>
#include <QDebug>

#include <cassert>
#include <KSharedConfig>

using namespace Kleo;
using namespace Kleo::Dialogs;

namespace
{
struct Line {
    QString attr;
    QString label;
    QString regex;
    QLineEdit *edit;
};
}

static QString pgpLabel(const QString &attr)
{
    if (attr == QLatin1String("NAME")) {
        return i18n("Name");
    }
    if (attr == QLatin1String("COMMENT")) {
        return i18n("Comment");
    }
    if (attr == QLatin1String("EMAIL")) {
        return i18n("EMail");
    }
    return QString();
}

static QString attributeLabel(const QString &attr, bool pgp)
{
    if (attr.isEmpty()) {
        return QString();
    }
    const QString label = /*pgp ?*/ pgpLabel(attr) /*: Kleo::DNAttributeMapper::instance()->name2label( attr )*/ ;
    if (!label.isEmpty())
        if (pgp) {
            return label;
        } else
            return i18nc("Format string for the labels in the \"Your Personal Data\" page",
                         "%1 (%2)", label, attr);
    else {
        return attr;
    }
}

static QString attributeFromKey(QString key)
{
    return key.remove(QLatin1Char('!'));
}

static int row_index_of(QWidget *w, QGridLayout *l)
{
    const int idx = l->indexOf(w);
    int r, c, rs, cs;
    l->getItemPosition(idx, &r, &c, &rs, &cs);
    return r;
}

static QLineEdit *adjust_row(QGridLayout *l, int row, const QString &label, const QString &preset, QValidator *validator, bool readonly, bool required)
{
    assert(l);
    assert(row >= 0);
    assert(row < l->rowCount());

    QLabel *lb = qobject_cast<QLabel *>(l->itemAtPosition(row, 0)->widget());
    assert(lb);
    QLineEdit *le = qobject_cast<QLineEdit *>(l->itemAtPosition(row, 1)->widget());
    assert(le);
    QLabel *reqLB = qobject_cast<QLabel *>(l->itemAtPosition(row, 2)->widget());
    assert(reqLB);

    lb->setText(i18nc("interpunctation for labels", "%1:", label));
    le->setText(preset);
    reqLB->setText(required ? i18n("(required)") : i18n("(optional)"));
    delete le->validator();
    if (validator) {
        if (!validator->parent()) {
            validator->setParent(le);
        }
        le->setValidator(validator);
    }

    le->setReadOnly(readonly && le->hasAcceptableInput());

    lb->show();
    le->show();
    reqLB->show();

    return le;
}

class AddUserIDDialog::Private
{
    friend class ::Kleo::Dialogs::AddUserIDDialog;
    AddUserIDDialog *const q;
public:
    explicit Private(AddUserIDDialog *qq)
        : q(qq),
          ui(q)
    {

    }

private:
    void slotUserIDChanged();

private:
    bool isComplete() const;

private:
    struct UI : public Ui_AddUserIDDialog {

        QVector<Line> lineList;

        explicit UI(AddUserIDDialog *qq)
            : Ui_AddUserIDDialog()
        {
            setupUi(qq);

            // ### this code is mostly the same as the one in
            // ### newcertificatewizard. Find some time to factor them
            // ### into a single copy.

            // hide the stuff
            nameLB->hide();
            nameLE->hide();
            nameRequiredLB->hide();

            emailLB->hide();
            emailLE->hide();
            emailRequiredLB->hide();

            commentLB->hide();
            commentLE->hide();
            commentRequiredLB->hide();

            // set errorLB to have a fixed height of two lines:
            errorLB->setText(QLatin1String("2<br>1"));
            errorLB->setFixedHeight(errorLB->minimumSizeHint().height());
            errorLB->clear();

            const KConfigGroup config(KSharedConfig::openConfig(), "CertificateCreationWizard");
            const QStringList attrOrder = config.readEntry("OpenPGPAttributeOrder",
                                          QStringList() << QLatin1String("NAME!") << QLatin1String("EMAIL!") << QLatin1String("COMMENT"));

            QMap<int, Line> lines;

            Q_FOREACH (const QString &rawKey, attrOrder) {
                const QString key = rawKey.trimmed().toUpper();
                const QString attr = attributeFromKey(key);
                if (attr.isEmpty()) {
                    continue;
                }
                const QString preset = config.readEntry(attr);
                const bool required = key.endsWith(QLatin1Char('!'));
                const bool readonly = config.isEntryImmutable(attr);
                const QString label = config.readEntry(attr + QLatin1String("_label"),
                                                       attributeLabel(attr, true));
                const QString regex = config.readEntry(attr + QLatin1String("_regex"));

                int row;
                QValidator *validator = 0;
                if (attr == QLatin1String("EMAIL")) {
                    validator = regex.isEmpty() ? Validation::email() : Validation::email(QRegExp(regex)) ;
                    row = row_index_of(emailLE, gridLayout);
                } else if (attr == QLatin1String("NAME")) {
                    validator = regex.isEmpty() ? Validation::pgpName() : Validation::pgpName(QRegExp(regex)) ;
                    row = row_index_of(nameLE, gridLayout);
                } else if (attr == QLatin1String("COMMENT")) {
                    validator = regex.isEmpty() ? Validation::pgpComment() : Validation::pgpComment(QRegExp(regex)) ;
                    row = row_index_of(commentLE, gridLayout);
                } else {
                    continue;
                }

                QLineEdit *le = adjust_row(gridLayout, row, label, preset, validator, readonly, required);

                const Line line = { key, label, regex, le };
                lines[row] = line;
            }

            lineList = kdtools::copy< QVector<Line> >(lines);
        }

        QPushButton *okPB() const
        {
            return buttonBox->button(QDialogButtonBox::Ok);
        }
    } ui;
};

AddUserIDDialog::AddUserIDDialog(QWidget *p, Qt::WindowFlags f)
    : QDialog(p, f), d(new Private(this))
{

}

AddUserIDDialog::~AddUserIDDialog() {}

void AddUserIDDialog::setName(const QString &name)
{
    d->ui.nameLE->setText(name);
}

QString AddUserIDDialog::name() const
{
    return d->ui.nameLE->text().trimmed();
}

void AddUserIDDialog::setEmail(const QString &email)
{
    d->ui.emailLE->setText(email);
}

QString AddUserIDDialog::email() const
{
    return d->ui.emailLE->text().trimmed();
}

void AddUserIDDialog::setComment(const QString &comment)
{
    d->ui.commentLE->setText(comment);
}

QString AddUserIDDialog::comment() const
{
    return d->ui.commentLE->text().trimmed();
}

static bool has_intermediate_input(const QLineEdit *le)
{
    QString text = le->text();
    int pos = le->cursorPosition();
    const QValidator *const v = le->validator();
    return !v || v->validate(text, pos) == QValidator::Intermediate ;
}

static bool requirementsAreMet(const QVector<Line> &list, QString &error)
{
    Q_FOREACH (const Line &line, list) {
        const QLineEdit *le = line.edit;
        if (!le) {
            continue;
        }
        const QString key = line.attr;
        qDebug() << "requirementsAreMet(): checking \"" << key << "\" against \"" << le->text() << "\":";
        if (le->text().trimmed().isEmpty()) {
            if (key.endsWith(QLatin1Char('!'))) {
                if (line.regex.isEmpty()) {
                    error = xi18nc("@info", "<interface>%1</interface> is required, but empty.", line.label);
                } else
                    error = xi18nc("@info", "<interface>%1</interface> is required, but empty.<nl/>"
                                   "Local Admin rule: <icode>%2</icode>", line.label, line.regex);
                return false;
            }
        } else if (has_intermediate_input(le)) {
            if (line.regex.isEmpty()) {
                error = xi18nc("@info", "<interface>%1</interface> is incomplete.", line.label);
            } else
                error = xi18nc("@info", "<interface>%1</interface> is incomplete.<nl/>"
                               "Local Admin rule: <icode>%2</icode>", line.label, line.regex);
            return false;
        } else if (!le->hasAcceptableInput()) {
            if (line.regex.isEmpty()) {
                error = xi18nc("@info", "<interface>%1</interface> is invalid.", line.label);
            } else
                error = xi18nc("@info", "<interface>%1</interface> is invalid.<nl/>"
                               "Local Admin rule: <icode>%2</icode>", line.label, line.regex);
            return false;
        }
        qDebug() << "ok" << endl;
    }
    return true;
}

bool AddUserIDDialog::Private::isComplete() const
{
    QString error;
    const bool ok = requirementsAreMet(ui.lineList, error);
    ui.errorLB->setText(error);
    return ok;
}

void AddUserIDDialog::Private::slotUserIDChanged()
{

    ui.okPB()->setEnabled(isComplete());

    const QString name = q->name();
    const QString email = q->email();
    const QString comment = q->comment();

    QStringList parts;
    if (!name.isEmpty()) {
        parts.push_back(name);
    }
    if (!comment.isEmpty()) {
        parts.push_back(QLatin1Char('(') + comment + QLatin1Char(')'));
    }
    if (!email.isEmpty()) {
        parts.push_back(QLatin1Char('<') + email + QLatin1Char('>'));
    }

    ui.resultLB->setText(parts.join(QLatin1String(" ")));
}

#include "moc_adduseriddialog.cpp"
