#include "gx-rnd-eng-engine.hpp"
#include "../../core/ecs/gx-cr-ecs-world.hpp"
#include "../../platform/gx-plt-application.hpp"
#include "../gltf/gx-rnd-gltf-loader.hpp"
#include "../scene/gx-rnd-scn-manager.hpp"

#ifdef GX_RENDER_VULKAN_ENABLED
#include "../../vulkan/engine/gx-vk-eng-engine.hpp"
#endif

#ifdef GX_RENDER_BGFX_ENABLED
#include "../../bgfx/gx-bgfx-engine.hpp"
#endif

#include <imgui.h>

gearoenix::render::engine::Engine::Engine(
    const Type engine_type,
    const platform::Application& platform_application) noexcept
    : engine_type(engine_type)
    , platform_application(platform_application)
    , scene_manager(new scene::Manager(this))
    , world(new core::ecs::World())
{
}

std::set<gearoenix::render::engine::Type> gearoenix::render::engine::Engine::get_available_engines() noexcept
{
    return {
#ifdef GX_RENDER_VULKAN_ENABLED
        Type::Vulkan,
#endif
#ifdef GX_RENDER_BGFX_ENABLED
        Type::BGFX,
#endif
    };
}

std::unique_ptr<gearoenix::render::engine::Engine> gearoenix::render::engine::Engine::construct(
    platform::Application& platform_application) noexcept
{
    std::unique_ptr<Engine> result;
    const auto& configuration = platform_application.get_base().get_configuration().get_render_configuration();
#ifdef GX_RENDER_VULKAN_ENABLED
    if (configuration.get_vulkan_render_backend_enabled() && vulkan::engine::Engine::is_supported()) {
        result = std::make_unique<vulkan::engine::Engine>(platform_application);
    }
#endif
#ifdef GX_RENDER_DIRECT3DX_ENABLED
    if (result == nullptr && configuration.get_direct3dx_render_backend_enabled() && direct3dx::engine::Engine::is_supported()) {
        result = direct3dx::engine::Engine::construct(configuration, std::move(platform_application));
    }
#endif
#ifdef GX_RENDER_METAL_ENABLED
    if (result == nullptr && configuration.get_metal_render_backend_enabled() && metal::engine::Engine::is_supported()) {
        result = metal::engine::Engine::construct(configuration, std::move(platform_application));
    }
#endif
#ifdef GX_RENDER_OPENGL_ENABLED
    if (result == nullptr && configuration.get_opengl_render_backend_enabled() && opengl::engine::Engine::is_supported()) {
        result = opengl::engine::Engine::construct(configuration, std::move(platform_application));
    }
#endif
#ifdef GX_RENDER_BGFX_ENABLED
    if (nullptr == result && configuration.get_bgfx_render_backend_enabled()) {
        result = std::make_unique<bgfx::Engine>(platform_application);
    }
#endif
    GX_CHECK_NOT_EQUAL(result, nullptr)
    return result;
}

gearoenix::render::engine::Engine::~Engine() noexcept = default;

void gearoenix::render::engine::Engine::start_frame() noexcept
{
    const std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> delta_time_duration = now - last_frame_time;
    delta_time = delta_time_duration.count();
    ImGui::GetIO().DeltaTime = static_cast<float>(delta_time);
    last_frame_time = now;
    previous_frame_number = frame_number_from_start % frames_count;
    ++frame_number_from_start;
    frame_number = frame_number_from_start % frames_count;
    next_frame_number = (frame_number_from_start + 1) % frames_count;
    world->update();
    // physics_engine->update();
}

void gearoenix::render::engine::Engine::end_frame() noexcept
{
}
