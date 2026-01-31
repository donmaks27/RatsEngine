#include <new>

void* operator new[](std::size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return ::operator new[](size);
}
void* operator new[](std::size_t size, std::size_t alignment, std::size_t offset, const char* pName, int flags, unsigned int debugFlags, const char* file, int line)
{
	return ::operator new[](size, static_cast<std::align_val_t>(alignment));
}