
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
CREATE TABLE "tblPalmExp" (
   "fldTdate" date NOT NULL,
   "fldAmount" money,
   "fldPType" text,
   "fldVName" text,
   "fldEType" text,
   "fldLocation" text,
   "fldAttendees" text,
   "fldNotes" text
);
CREATE  INDEX "tblPalmExp_fldTdate_key" ON "tblPalmExp" ("fldTdate");
