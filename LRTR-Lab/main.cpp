#include "Runtimes/LabApp.hpp"

#include <iostream>

int main() {
	LRTR::LabApp app = LRTR::LabApp("LRTR-Lab", 1920, 1080);
	
	app.show();
	app.runLoop();
}