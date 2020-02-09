#include "LabApp.hpp"

int main() {
	LRTR::LabApp app = LRTR::LabApp({ "LRTR-Lab", 1280, 720, 12 });

	app.show();
	app.runLoop();
}