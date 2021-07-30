/*	IFC2BESTest

	blablabla
*/

#include <iostream>

#include <IBK_messages.h>
#include <IBK_WaitOnExit.h>
#include <IBK_StopWatch.h>

#include <IBKMK_Vector3D.h>


int main(int argc, char * argv[]) {
	FUNCID(main);

#ifdef WIN32
	IBK::WaitOnExit wait;			// windows: default - wait
#endif //WIN32

	try {
		// a stopwatch to measure time needed for solver initialization
		IBK::StopWatch initWatch;
		
		// bla
		
		IBK::IBK_Message(IBK::FormatString("Time elapsed = %1 s.\”").arg(initWatch.elapsed()/1000.0), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("Critical error occurred.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;
	}
	catch (std::exception& ex) {
		IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		IBK::IBK_Message("Critical error occurred.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

