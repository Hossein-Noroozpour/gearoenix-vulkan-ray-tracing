#include "gx-plt-application.hpp"
#include "gx-plt-log.hpp"

gearoenix::platform::BaseApplication::BaseApplication(const RuntimeConfiguration& configuration) noexcept
    : configuration(configuration)
{
    if (!configuration.get_fullscreen()) {
        initialize_window_size(
            static_cast<decltype(window_width)>(configuration.get_window_width()),
            static_cast<decltype(window_height)>(configuration.get_window_height()));
    }
}

void gearoenix::platform::BaseApplication::initialize_window_position(
    const int x, const int y) noexcept
{
    pre_window_x = window_x = x;
    pre_window_y = window_y = y;
    GX_TODO
}

void gearoenix::platform::BaseApplication::initialize_window_size(
    const int w, const int h) noexcept
{
    pre_window_width = window_width = w;
    pre_window_height = window_height = h;
    GX_TODO
}

void gearoenix::platform::BaseApplication::initialize_mouse_position(
    const double x, const double y) noexcept
{
    pre_mouse_x = mouse_x = x;
    pre_mouse_y = mouse_y = y;
    GX_TODO
}

void gearoenix::platform::BaseApplication::going_to_be_closed() noexcept
{
    running = false;
    GX_TODO
}