#include "LabApp.hpp"

#ifdef __RESOLUTION_2K__
const LRTR::AppStartup startup = { "LRTR-Lab", 2560, 1440, 25 };
#else
#ifdef __RESOLUTION_1080P__
const LRTR::AppStartup startup = { "LRTR-Lab", 1920, 1080, 20 };
#else
const LRTR::AppStartup startup = { "LRTR-Lab", 1280, 720, 12 };
#endif
#endif

int main() {
	
	LRTR::LabApp app = LRTR::LabApp(startup);

	app.show();
	app.runLoop();
}