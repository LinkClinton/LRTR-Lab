#include "Runtimes/LabApp.hpp"

#include "Scenes/Transform.hpp"

int main() {
	LRTR::LabApp app = LRTR::LabApp("LRTR-Lab", 1920, 1080);

	app.show();
	app.runLoop();
}