#include "FormatHandler.h"

FormatHandler::FormatHandler()
  : QXmlDefaultHandler()
{
  // Empty.
}

FormatHandler::~FormatHandler()
{
  // Empty.
}


  FormatSpec
FormatHandler::spec()
{
  return spec_;
}


  bool
FormatHandler::startDocument()
{
  return true; // Nothing to do yet.
}


  bool
FormatHandler::startElement
(
 const QString &,
 const QString &,
 const QString & qName,
 const QXmlAttributes & attributes
)
{
  if ("kab_format" == qName)
  {
    name_ = attributes.value("name");

    return true;
  }
  else if ("field" == qName)
  {
    FieldFormat ff;

    ff.setName(attributes.value("name"));
    ff.setUnique("true" == attributes.value("unique"));
    ff.setType(attributes.value("type"));
    ff.setSubType(attributes.value("subtype"));

    spec_ << ff;

    return true;
  }
  else
  {
    return false;
  }
}


  bool
FormatHandler::endElement
(
 const QString &,
 const QString &,
 const QString &
)
{
  // What to do here ?
  return true;
}

