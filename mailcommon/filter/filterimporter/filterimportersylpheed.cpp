/*
  Copyright (c) 2012, 2013 Montel Laurent <montel@kde.org>

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

#include "filterimportersylpheed_p.h"
#include "filtermanager.h"
#include "mailfilter.h"

#include <KDebug>

#include <QFile>
#include <QDir>

using namespace MailCommon;

FilterImporterSylpheed::FilterImporterSylpheed( QFile *file )
    :FilterImporterAbstract()
{
    QDomDocument doc;
    if ( !loadDomElement( doc, file ) ) {
        return;
    }
    QDomElement filters = doc.documentElement();

    if ( filters.isNull() ) {
        kDebug() << "No filters defined";
        return;
    }

    for ( QDomElement e = filters.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
        const QString tag = e.tagName();
        if ( tag == QLatin1String( "rule" ) ) {
            parseFilters(e);
        } else {
            kDebug() << " unknown tag " << tag;
        }
    }
}

FilterImporterSylpheed::~FilterImporterSylpheed()
{
}

QString FilterImporterSylpheed::defaultFiltersSettingsPath()
{
    return QString::fromLatin1( "%1/.sylpheed-2.0/filter.xml" ).arg( QDir::homePath() );
}

void FilterImporterSylpheed::parseConditions( const QDomElement &e, MailCommon::MailFilter *filter )
{
    if ( e.hasAttribute( QLatin1String("bool") ) ) {
        const QString attr = e.attribute( QLatin1String("bool") );
        if ( attr == QLatin1String( "and" ) ) {
            filter->pattern()->setOp( SearchPattern::OpAnd );
        } else if ( attr == QLatin1String( "or" ) ) {
            filter->pattern()->setOp( SearchPattern::OpOr );
        } else {
            kDebug() << " bool not defined: " << attr;
        }
    }
    for ( QDomElement ruleFilter = e.firstChildElement();
          !ruleFilter.isNull();
          ruleFilter = ruleFilter.nextSiblingElement() ) {
        QString contentsName;
        QByteArray fieldName;
        SearchRule::Function functionName = SearchRule::FuncNone;

        const QString nexttag = ruleFilter.tagName();
        if ( nexttag == QLatin1String( "match-header" ) ){
            if ( ruleFilter.hasAttribute( QLatin1String("name") ) ) {
                const QString attr = ruleFilter.attribute( QLatin1String("name") );
                if ( attr == QLatin1String( "From" ) ) {
                    fieldName = "from";
                } else if ( attr == QLatin1String( "Cc" ) ) {
                    fieldName = "cc";
                } else if ( attr == QLatin1String( "To" ) ) {
                    fieldName = "to";
                } else if ( attr == QLatin1String( "Reply-To" ) ) {
                    fieldName = "reply-to";
                } else if ( attr == QLatin1String( "Subject" ) ) {
                    fieldName = "subject";
                } else if ( attr == QLatin1String( "List-Id" ) ) {
                    fieldName = "list-id";
                } else if ( attr == QLatin1String( "X-ML-Name" ) ) {
                    fieldName = "x-mailing-list";
                }
                if ( fieldName.isEmpty() ) {
                    kDebug()<<" match-header not implemented " << attr;
                }
            }
            contentsName = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "match-any-header" ) ) {
            fieldName = "<any header>";
            contentsName = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "match-to-or-cc" ) ) {
            fieldName = "<recipients>";
            contentsName = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "match-body-text" ) ) {
            fieldName = "<body>";
            contentsName = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "command-test" ) ) {
            //TODO
            //Not implemented in kmail
        } else if ( nexttag == QLatin1String( "size" ) ) {
            fieldName = "<size>";
            contentsName = QString::number(ruleFilter.text().toInt()*1024); //Stored as kb
        } else if ( nexttag == QLatin1String( "age" ) ) {
            fieldName = "<age in days>";
            contentsName = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "unread" ) ) {
            fieldName = "<status>";
            contentsName = QLatin1String( "Unread" );
        } else if ( nexttag == QLatin1String( "mark" ) ) {
            //TODO
        } else if ( nexttag == QLatin1String( "color-label" ) ) {
            //TODO
        } else if ( nexttag == QLatin1String( "mime" ) ) {
            //TODO
        } else if ( nexttag == QLatin1String( "account-id" ) ) {
            //TODO
        } else if ( nexttag == QLatin1String( "target-folder" ) ) {
            //TODO
        } else {
            kDebug() << " tag not recognize " << nexttag;
        }
        if (fieldName.isEmpty()) {
            kDebug()<<" field not implemented "<<nexttag;
        }

        if ( ruleFilter.hasAttribute( QLatin1String("type") ) ) {
            const QString attr = ruleFilter.attribute( QLatin1String("type") );
            if ( attr == QLatin1String( "not-contain" ) ) {
                functionName = SearchRule::FuncContainsNot;
            } else if ( attr == QLatin1String( "contains" ) ) {
                functionName = SearchRule::FuncContains;
            } else if ( attr == QLatin1String( "is-not" ) ) {
                functionName = SearchRule::FuncNotEqual;
            } else if ( attr == QLatin1String( "is" ) ) {
                functionName = SearchRule::FuncEquals;
            } else if ( attr == QLatin1String( "not-regex" ) ) {
                functionName = SearchRule::FuncNotRegExp;
            } else if ( attr == QLatin1String( "regex" ) ) {
                functionName = SearchRule::FuncRegExp;
            } else if ( attr == QLatin1String( "not-in-addressbook" ) ) {
                functionName = SearchRule::FuncIsNotInAddressbook;
            } else if ( attr == QLatin1String( "in-addressbook" ) ) {
                functionName = SearchRule::FuncIsInAddressbook;
            } else if ( attr == QLatin1String( "gt" ) ) {
                functionName = SearchRule::FuncIsGreater;
            } else if ( attr == QLatin1String( "lt" ) ) {
                functionName = SearchRule::FuncIsLess;
            } else {
                kDebug() << " Attr type not implemented :" << attr;
            }
        }
        SearchRule::Ptr rule = SearchRule::createInstance( fieldName, functionName, contentsName );
        filter->pattern()->append( rule );
    }
}

void FilterImporterSylpheed::parseActions( const QDomElement &e,
                                           MailCommon::MailFilter *filter )
{
    for ( QDomElement ruleFilter = e.firstChildElement();
          !ruleFilter.isNull();
          ruleFilter = ruleFilter.nextSiblingElement() ) {
        QString actionName;
        const QString nexttag = ruleFilter.tagName();
        QString value = ruleFilter.text();
        if ( nexttag == QLatin1String( "move" ) ){
            actionName = QLatin1String( "transfer" );
            value = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "copy" ) ) {
            actionName = QLatin1String( "copy" );
            value = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "not-receive" ) ) {
            //TODO
        } else if ( nexttag == QLatin1String( "delete" ) ) {
            actionName = QLatin1String( "delete" );
        } else if ( nexttag == QLatin1String( "exec" ) ) {
            actionName = QLatin1String( "execute" );
            value = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "exec-async" ) ) {
            actionName = QLatin1String( "filter app" );
            value = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "mark" ) ) {
            //FIXME add tag ?
        } else if ( nexttag == QLatin1String( "color-label" ) ) {
            //TODO
        } else if ( nexttag == QLatin1String( "mark-as-read" ) ) {
            actionName = QLatin1String( "set status" );
            value = QLatin1String( "R" );
        } else if ( nexttag == QLatin1String( "forward" ) ) {
            actionName = QLatin1String( "forward" );
            value = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "forward-as-attachment" ) ) {
            //TODO
        } else if ( nexttag == QLatin1String( "redirect" ) ) {
            actionName = QLatin1String( "redirect" );
            value = ruleFilter.text();
        } else if ( nexttag == QLatin1String( "stop-eval" ) ) {
            filter->setStopProcessingHere( true );
            break;
        }

        if ( actionName.isEmpty() ) {
            kDebug() << " tag not recognize " << nexttag;
        }
        createFilterAction( filter, actionName, value );
    }
}

void FilterImporterSylpheed::parseFilters( const QDomElement &e )
{
    MailCommon::MailFilter *filter = new MailCommon::MailFilter();
    if ( e.hasAttribute( QLatin1String("enabled") ) ) {
        const QString attr = e.attribute( QLatin1String("enabled") );
        if ( attr == QLatin1String( "false" ) ) {
            filter->setEnabled( false );
        }
    }

    if ( e.hasAttribute( QLatin1String("name") ) ) {
        const QString attr = e.attribute( QLatin1String("name") );
        filter->pattern()->setName( attr );
        filter->setToolbarName( attr );
    }

    if ( e.hasAttribute( QLatin1String("timing") ) ) {
        const QString attr = e.attribute( QLatin1String("timing") );
        if ( attr == QLatin1String( "any" ) ) {
            filter->setApplyOnInbound( true );
            filter->setApplyOnExplicit( true );
        } else if ( attr == QLatin1String( "receiver" ) ) {
            filter->setApplyOnInbound( true );
        } else if ( attr == QLatin1String( "manual" ) ) {
            filter->setApplyOnInbound( false );
            filter->setApplyOnExplicit( true );
        } else {
            kDebug() << " timing not defined: " << attr;
        }

    }
    for ( QDomElement ruleFilter = e.firstChildElement();
          !ruleFilter.isNull();
          ruleFilter = ruleFilter.nextSiblingElement() ) {
        const QString nexttag = ruleFilter.tagName();
        if ( nexttag == QLatin1String( "condition-list" ) ){
            parseConditions( ruleFilter, filter );
        } else if ( nexttag == QLatin1String( "action-list" ) ) {
            parseActions( ruleFilter, filter );
        } else {
            kDebug() << " next tag not implemented " << nexttag;
        }
    }

    appendFilter(filter);
}
