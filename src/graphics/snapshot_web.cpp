#include "graphics/snapshot.h"

namespace solace::snapshot
{
void request() {}
bool ready()
{
    return false;
}
ImTextureID texture()
{
    return ImTextureID_Invalid;
}
void poll(ID3D11Device*, ID3D11DeviceContext*, IDXGISwapChain*) {}
void release() {}
void shutdown() {}
void attach(ID3D11Device*, ID3D11DeviceContext*, IDXGISwapChain*) {}
void capture_backdrop(ImDrawList*) {}
void invalidate_backdrop() {}
bool backdrop_ready()
{
    return false;
}
ImTextureID backdrop()
{
    return ImTextureID_Invalid;
}
} // namespace solace::snapshot
