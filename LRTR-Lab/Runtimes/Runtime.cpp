#include "Runtime.hpp"

#include "../Scenes/Component.hpp"
#include "../Scenes/Shape.hpp"

void LRTR::Runtime::initialize()
{
	Component::initialize();
	Shape::initialize();
}
