#include "gx-vk-msh-accel.hpp"
#ifdef GX_RENDER_VULKAN_ENABLED
#include "../../core/ecs/gx-cr-ecs-entity.hpp"
#include "../../core/macro/gx-cr-mcr-zeroer.hpp"
#include "../buffer/gx-vk-buf-buffer.hpp"
#include "../engine/gx-vk-eng-engine.hpp"
#include "gx-vk-msh-accel-component.hpp"

gearoenix::vulkan::mesh::Accel::Accel(
    std::shared_ptr<buffer::Buffer> vertex,
    std::shared_ptr<buffer::Buffer> index) noexcept
    : vertex(std::move(vertex))
    , index(std::move(index))
{
}

void gearoenix::vulkan::mesh::Accel::initialize_blas() noexcept
{
    VkAccelerationStructureDeviceAddressInfoKHR info;
    GX_SET_ZERO(info)
    info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    info.accelerationStructure = vulkan_data;
    acceleration_address = vkGetAccelerationStructureDeviceAddressKHR(
        vertex->get_allocated_memory()->get_e().get_logical_device().get_vulkan_data(), &info);
}

std::shared_ptr<gearoenix::vulkan::mesh::Accel> gearoenix::vulkan::mesh::Accel::construct(
    engine::Engine& e,
    const std::vector<math::BasicVertex>& vertices,
    const std::vector<std::uint32_t>& indices,
    core::sync::EndCaller<Accel>& c) noexcept
{
    core::sync::EndCaller<buffer::Buffer> end([c](const auto&) {});
    auto& buf_mgr = e.get_buffer_manager();
    auto vertex = buf_mgr.create(vertices, end);
    auto index = buf_mgr.create(indices, std::move(end));
    std::shared_ptr<Accel> result(new Accel(std::move(vertex), std::move(index)));
    result->self = result;
    c.set_data(result);
    return result;
}

gearoenix::vulkan::mesh::Accel::~Accel() noexcept
{
    vkDestroyAccelerationStructureKHR(
        vertex->get_allocated_memory()->get_e().get_logical_device().get_vulkan_data(), vulkan_data, nullptr);
    vulkan_data = nullptr;
}

void gearoenix::vulkan::mesh::Accel::set_component(const std::shared_ptr<core::ecs::EntitySharedBuilder>& b) noexcept
{
    b->get_builder().add_component(AccelComponent(self.lock()));
}

std::pair<VkDeviceAddress, VkDeviceAddress> gearoenix::vulkan::mesh::Accel::get_buffers_address() const noexcept
{
    return {
        vertex->get_device_address(),
        index->get_device_address(),
    };
}

#endif