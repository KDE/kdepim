#ifndef __STATUS_MESSAGES_H
#define __STATUS_MESSAGES_H

#include <unistd.h>

#define KPILOTLINK_PORT  10159

class CStatusMessages
{
public:
	typedef enum LinkMessages {
		CONDUIT_READY=1,
		SYNC_STARTING,
		SYNC_COMPLETED,
		SYNCING_DATABASE,
		RECORD_MODIFIED,
		RECORD_DELETED,
		FILE_INSTALL_REQUEST, // Used between kpilotDaemon and kpilot
		NEW_RECORD_ID,
		WRITE_RECORD,
		NEXT_MODIFIED_REC,
		NEXT_REC_IN_CAT,
		READ_REC_BY_INDEX,
		READ_REC_BY_ID,
		NO_SUCH_RECORD,
		REC_DATA,
		LOG_MESSAGE
	} ;

	static int write(int fd,LinkMessages m)
	{
		int i = m;
		return ::write(fd,&i,sizeof(int));
	}
};


#endif
