#ifndef __STATUS_MESSAGES_H
#define __STATUS_MESSAGES_H

#define KPILOTLINK_PORT  10159

class CStatusMessages
{
public:
  static const int CONDUIT_READY;
  static const int SYNC_STARTING;
  static const int SYNC_COMPLETED;
  static const int SYNCING_DATABASE;
  static const int RECORD_MODIFIED;
  static const int RECORD_DELETED;
  static const int FILE_INSTALL_REQUEST; // Used between kpilotDaemon and kpilot

  static const int NEW_RECORD_ID;
  static const int WRITE_RECORD;
  static const int NEXT_MODIFIED_REC;
  static const int NEXT_REC_IN_CAT;
  static const int READ_REC_BY_INDEX;
  static const int READ_REC_BY_ID;
  static const int NO_SUCH_RECORD;
  static const int REC_DATA;
};


#endif
