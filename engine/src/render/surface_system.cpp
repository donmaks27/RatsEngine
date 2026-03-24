#include <engine/render/surface_system.h>

namespace engine
{
	const log::logger surface_system::Log = surface_system::logger();
	surface_system* surface_system::Instance = nullptr;
}
