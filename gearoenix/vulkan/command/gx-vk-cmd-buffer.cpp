#include "gx-vk-cmd-buffer.hpp"
#ifdef GX_RENDER_VULKAN_ENABLED
#include "../../core/macro/gx-cr-mcr-zeroer.hpp"
#include "../buffer/gx-vk-buf-buffer.hpp"
#include "../device/gx-vk-dev-logical.hpp"
#include "../device/gx-vk-dev-physical.hpp"
#include "../gx-vk-check.hpp"
#include "../gx-vk-framebuffer.hpp"
#include "../gx-vk-render-pass.hpp"
#include "../image/gx-vk-img-view.hpp"
#include "../sync/gx-vk-sync-fence.hpp"
#include "gx-vk-cmd-pool.hpp"

#ifdef GX_DEBUG_MODE
#define GX_VK_CMD_BUFF_DEBUG
#endif

gearoenix::vulkan::command::Buffer::Buffer(Pool* const pool, const Type t) noexcept
    : pool(pool)
    , type(t)
{
    VkCommandBufferAllocateInfo info;
    GX_SET_ZERO(info)
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = pool->get_vulkan_data();
    info.level = t == Type::Primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    info.commandBufferCount = 1;
    GX_VK_CHK(vkAllocateCommandBuffers(pool->get_logical_device().get_vulkan_data(), &info, &vulkan_data))
}

gearoenix::vulkan::command::Buffer::Buffer(command::Buffer&& o) noexcept
    : pool(o.pool)
    , type(o.type)
    , vulkan_data(o.vulkan_data)
{
    o.vulkan_data = nullptr;
}

gearoenix::vulkan::command::Buffer::~Buffer() noexcept
{
    if (nullptr != vulkan_data) {
        vkFreeCommandBuffers(
            pool->get_logical_device().get_vulkan_data(),
            pool->get_vulkan_data(), 1, &vulkan_data);
        vulkan_data = nullptr;
    }
}

void gearoenix::vulkan::command::Buffer::begin() noexcept
{
    VkCommandBufferBeginInfo info;
    GX_SET_ZERO(info)
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    GX_VK_CHK(vkBeginCommandBuffer(vulkan_data, &info))
}

void gearoenix::vulkan::command::Buffer::flush() noexcept
{
    sync::Fence fence(pool->get_logical_device());
    GX_VK_CHK(vkEndCommandBuffer(vulkan_data))
    VkSubmitInfo submit_info;
    GX_SET_ZERO(submit_info)
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vulkan_data;
    GX_VK_CHK(vkQueueSubmit(
        pool->get_logical_device().get_graphic_queue(),
        1, &submit_info, fence.get_vulkan_data()))
    fence.wait();
}

void gearoenix::vulkan::command::Buffer::end() noexcept
{
    GX_VK_CHK(vkEndCommandBuffer(vulkan_data))
}

void gearoenix::vulkan::command::Buffer::copy(
    buffer::Buffer& src,
    buffer::Buffer& des,
    const std::vector<VkBufferCopy>& info) noexcept
{
    vkCmdCopyBuffer(vulkan_data, src.get_vulkan_data(), des.get_vulkan_data(), info.size(), info.data());
}

void gearoenix::vulkan::command::Buffer::begin(const RenderPass& render_pass, const Framebuffer& framebuffer) noexcept
{
    const auto& img = framebuffer.get_depth()->get_image();
    VkRenderPassBeginInfo info;
    GX_SET_ZERO(info)
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = render_pass.get_vulkan_data();
    info.framebuffer = framebuffer.get_vulkan_data();
    info.renderArea.extent.width = img.get_image_width();
    info.renderArea.extent.height = img.get_image_height();
    info.clearValueCount = 2;
    info.pClearValues = framebuffer.get_clear_colors().data();
    vkCmdBeginRenderPass(vulkan_data, &info, VK_SUBPASS_CONTENTS_INLINE);
}

//void gearoenix::vulkan::command::Buffer::begin_render_pass_with_info(
//    const VkRenderPassBeginInfo& render_pass_begin_info) noexcept
//{
//    Loader::vkCmdBeginRenderPass(vulkan_data, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
//}
//
//void gearoenix::vulkan::command::Buffer::set_viewport(
//    const VkViewport& viewport) noexcept
//{
//    Loader::vkCmdSetViewport(vulkan_data, 0, 1, &viewport);
//}
//
//void gearoenix::vulkan::command::Buffer::set_scissor(const VkRect2D& scissor) noexcept
//{
//    Loader::vkCmdSetScissor(vulkan_data, 0, 1, &scissor);
//}
//

void gearoenix::vulkan::command::Buffer::end_render_pass() noexcept
{
    vkCmdEndRenderPass(vulkan_data);
}

//void gearoenix::vulkan::command::Buffer::bind_pipeline(VkPipeline pip) noexcept
//{
//    Loader::vkCmdBindPipeline(vulkan_data, VK_PIPELINE_BIND_POINT_GRAPHICS, pip);
//}
//
//void gearoenix::vulkan::command::Buffer::bind_descriptor_set(
//    VkPipelineBindPoint pipeline_bind_point,
//    VkPipelineLayout pipeline_layout,
//    uint32_t first_set,
//    const VkDescriptorSet& des_set) noexcept
//{
//    Loader::vkCmdBindDescriptorSets(
//        vulkan_data, pipeline_bind_point, pipeline_layout, first_set, 1, &des_set, 0, nullptr);
//}
//
//void gearoenix::vulkan::command::Buffer::bind_vertex_buffers(VkBuffer buf, VkDeviceSize offset) noexcept
//{
//    Loader::vkCmdBindVertexBuffers(vulkan_data, 0, 1, &buf, &offset);
//}
//
//void gearoenix::vulkan::command::Buffer::bind_index_buffer(VkBuffer buf, VkDeviceSize offset) noexcept
//{
//    Loader::vkCmdBindIndexBuffer(vulkan_data, buf, offset, VK_INDEX_TYPE_UINT32);
//}

void gearoenix::vulkan::command::Buffer::draw_indices(VkDeviceSize count) noexcept
{
    vkCmdDrawIndexed(vulkan_data, static_cast<std::uint32_t>(count), 1, 0, 0, 1);
}

const VkCommandBuffer* gearoenix::vulkan::command::Buffer::get_vulkan_data_ptr() const noexcept
{
    return &vulkan_data;
}

#endif
