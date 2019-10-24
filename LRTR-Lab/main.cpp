#pragma warning(disable : 4275)

#include "LRTRApp\LRTRApp.hpp"

int main() {
	LRTR::LRTRApp app = LRTR::LRTRApp("LRTR-Lab", 1920, 1080);

	app.show();
	app.runLoop();
}