class SyncManagerIface_stub;
class PilotDatabase;

class BackupConduit
{
public:
	BackupConduit(SyncManagerIface_stub *s) :
		fStub(s)
	{ } ;

	bool exec(PilotDatabase *);

private:
	SyncManagerIface_stub *fStub;
} ;
