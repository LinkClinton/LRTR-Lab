#include "Runtimes/Runtime.hpp"
#include "Runtimes/LabApp.hpp"

int main() {
	LRTR::LabApp app = LRTR::LabApp("LRTR-Lab", 1920, 1080);
	
	app.show();
	app.runLoop();
}