
/* -------------------------------------------------------- 
  phpPgAdmin 2.2.1 DB Dump
  http://www.greatbridge.org/project/phppgadmin/
  Host: localhost:5432
  Database : palm
  2001-16-03 11:03:44
-------------------------------------------------------- */ 
/* No Sequences found */
/* -------------------------------------------------------- 
  Table structure for table "tblPalmExp" 
-------------------------------------------------------- */
CREATE TABLE "tblpalmexp" (
   "fldtdate" date NOT NULL,
   "fldamount" money,
   "fldptype" text,
   "fldvname" text,
   "fldetype" text,
   "fldlocation" text,
   "fldattendees" text,
   "fldnotes" text
);
CREATE  INDEX "tblpalmexp_fldtdate_key" ON "tblpalmexp" ("fldtdate");
