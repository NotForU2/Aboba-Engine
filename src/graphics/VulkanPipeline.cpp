#include "VulkanPipeline.hpp"
#include <fstream>
#include <array>

std::vector<char> VulkanPipeline::ReadFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule VulkanPipeline::CreateShaderModule(const VulkanContext &context, const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(context.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module");
    }

    return shaderModule;
}

void VulkanPipeline::Create(VulkanContext &context, const std::string &vertFile, const std::string &fragFile, VkDescriptorSetLayout descriptorSetLayout, VkFormat colorAttachmentFormat, VkFormat depthAttachmentFormat)
{
    auto vertShaderCode = ReadFile(vertFile);
    auto fragShaderCode = ReadFile(fragFile);

    VkShaderModule vertShaderModule = CreateShaderModule(context, vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(context, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
        vertShaderStageInfo,
        fragShaderStageInfo,
    };

    // 1. Vertex Input
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    // 2. Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    // 3. Dynamic States (Viewport и Scissor)
    std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    // 4. Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    // 5. Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    // 6. Depth Stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    // 7. Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    // 8. Push Constant
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(MeshPushConstants),
    };

    // 9. Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange,
    };

    if (vkCreatePipelineLayout(context.GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    // 10. Dynamic Rendering Info
    VkPipelineRenderingCreateInfo pipelineRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorAttachmentFormat,
        .depthAttachmentFormat = depthAttachmentFormat,
    };

    // 11. Build
    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipelineRenderingInfo,
        .stageCount = 2,
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = mPipelineLayout,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
    };

    if (vkCreateGraphicsPipelines(context.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    std::cout << "Vulkan Graphics Pipeline created successfully" << std::endl;

    // 12. Delete Shader modules
    vkDestroyShaderModule(context.GetDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(context.GetDevice(), vertShaderModule, nullptr);
}

void VulkanPipeline::Destroy(const VulkanContext &context)
{
    if (mPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(context.GetDevice(), mPipeline, nullptr);
        mPipeline = VK_NULL_HANDLE;
    }
    if (mPipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(context.GetDevice(), mPipelineLayout, nullptr);
        mPipelineLayout = VK_NULL_HANDLE;
    }
}

void VulkanPipeline::Bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
}