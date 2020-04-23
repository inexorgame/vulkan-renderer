#include "inexor/vulkan-renderer/gltf-model/manager.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf/tiny_gltf.h>

namespace inexor::vulkan_renderer::gltf_model {

VkResult Manager::init(const VkDevice &device, const std::shared_ptr<VulkanTextureManager> texture_manager,
                       const std::shared_ptr<UniformBufferManager> uniform_buffer_manager, const std::shared_ptr<MeshBufferManager> mesh_buffer_manager,
                       const std::shared_ptr<DescriptorManager> descriptor_manager) {
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(descriptor_manager);

    // TODO: Mutex here!

    spdlog::debug("Initialising glTF 2.0 model manager.");

    this->device = device;
    this->texture_manager = texture_manager;
    this->uniform_buffer_manager = uniform_buffer_manager;
    this->mesh_buffer_manager = mesh_buffer_manager;
    this->descriptor_manager = descriptor_manager;

    model_manager_initialised = true;

    return VK_SUCCESS;
}

void Manager::load_node(std::shared_ptr<ModelNode> parent, const tinygltf::Node &node, uint32_t node_index, std::shared_ptr<Model> model, float globalscale) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(model);
    assert(globalscale > 0.0f);

    std::shared_ptr<ModelNode> newNode = std::make_shared<ModelNode>();

    newNode->index = node_index;
    newNode->parent = parent;
    newNode->name = node.name;
    newNode->skinIndex = node.skin;
    newNode->matrix = glm::mat4(1.0f);

    // Generate local node matrix
    glm::vec3 translation = glm::vec3(0.0f);

    if (3 == node.translation.size()) {
        translation = glm::make_vec3(node.translation.data());
        newNode->translation = translation;
    }

    glm::mat4 rotation = glm::mat4(1.0f);

    if (4 == node.rotation.size()) {
        glm::quat q = glm::make_quat(node.rotation.data());
        newNode->rotation = glm::mat4(q);
    }

    glm::vec3 scale = glm::vec3(1.0f);

    if (3 == node.scale.size()) {
        scale = glm::make_vec3(node.scale.data());
        newNode->scale = scale;
    }

    if (16 == node.matrix.size()) {
        newNode->matrix = glm::make_mat4x4(node.matrix.data());
    };

    // Node with children
    if (node.children.size() > 0) {
        for (std::size_t i = 0; i < node.children.size(); i++) {
            load_node(newNode, model->gltf2_container.nodes[node.children[i]], node.children[i], model, globalscale);
        }
    }

    // Node contains mesh data.
    if (node.mesh > -1) {
        const tinygltf::Mesh mesh = model->gltf2_container.meshes[node.mesh];

        std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>();

        std::string gltf_model_name = "glTF 2.0 model '" + model->name + "', Node: " + std::to_string(model->uniform_buffer_index) + ".";

        model->uniform_buffer_index++;

        // Allocate uniform buffers!
        uniform_buffer_manager->create_uniform_buffer(gltf_model_name, sizeof(StandardUniformBufferBlock), newMesh->uniform_buffer);

        // newMesh->set_matrix(newNode->matrix);

        for (std::size_t j = 0; j < mesh.primitives.size(); j++) {
            const tinygltf::Primitive &primitive = mesh.primitives[j];

            uint32_t index_start = static_cast<uint32_t>(model->index_buffer_cache.size());
            uint32_t vertex_start = static_cast<uint32_t>(model->vertex_buffer_cache.size());

            uint32_t index_count = 0;
            uint32_t vertex_count = 0;

            glm::vec3 posMin{};
            glm::vec3 posMax{};

            bool hasSkin = false;
            bool hasIndices = primitive.indices > -1;

            // Vertices
            {
                const float *bufferPos = nullptr;
                const float *bufferNormals = nullptr;
                const float *bufferTexCoordSet0 = nullptr;
                const float *bufferTexCoordSet1 = nullptr;
                const uint16_t *bufferJoints = nullptr;
                const float *bufferWeights = nullptr;

                int posByteStride;
                int normByteStride;
                int uv0ByteStride;
                int uv1ByteStride;
                int jointByteStride;
                int weightByteStride;

                // Position attribute is required.
                assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

                const tinygltf::Accessor &posAccessor = model->gltf2_container.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView &posView = model->gltf2_container.bufferViews[posAccessor.bufferView];

                bufferPos =
                    reinterpret_cast<const float *>(&(model->gltf2_container.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));

                posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
                posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

                vertex_count = static_cast<uint32_t>(posAccessor.count);

                posByteStride =
                    posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    const tinygltf::Accessor &normAccessor = model->gltf2_container.accessors[primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView &normView = model->gltf2_container.bufferViews[normAccessor.bufferView];
                    bufferNormals =
                        reinterpret_cast<const float *>(&(model->gltf2_container.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
                    normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float))
                                                                       : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor &uvAccessor = model->gltf2_container.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView &uvView = model->gltf2_container.bufferViews[uvAccessor.bufferView];
                    bufferTexCoordSet0 =
                        reinterpret_cast<const float *>(&(model->gltf2_container.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
                    uv0ByteStride =
                        uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
                }

                if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
                    const tinygltf::Accessor &uvAccessor = model->gltf2_container.accessors[primitive.attributes.find("TEXCOORD_1")->second];
                    const tinygltf::BufferView &uvView = model->gltf2_container.bufferViews[uvAccessor.bufferView];
                    bufferTexCoordSet1 =
                        reinterpret_cast<const float *>(&(model->gltf2_container.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
                    uv1ByteStride =
                        uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
                }

                // Skinning
                // Joints
                if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor &jointAccessor = model->gltf2_container.accessors[primitive.attributes.find("JOINTS_0")->second];
                    const tinygltf::BufferView &jointView = model->gltf2_container.bufferViews[jointAccessor.bufferView];
                    bufferJoints = reinterpret_cast<const uint16_t *>(
                        &(model->gltf2_container.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]));
                    jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / sizeof(bufferJoints[0]))
                                                                          : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
                }

                if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor &weightAccessor = model->gltf2_container.accessors[primitive.attributes.find("WEIGHTS_0")->second];
                    const tinygltf::BufferView &weightView = model->gltf2_container.bufferViews[weightAccessor.bufferView];
                    bufferWeights = reinterpret_cast<const float *>(
                        &(model->gltf2_container.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
                    weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float))
                                                                             : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
                }

                hasSkin = (bufferJoints && bufferWeights);

                for (std::size_t v = 0; v < posAccessor.count; v++) {
                    ModelVertex vert{};

                    vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
                    vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
                    vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
                    vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);

                    vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * jointByteStride])) : glm::vec4(0.0f);
                    vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);

                    // Fix for all zero weights
                    if (glm::length(vert.weight0) == 0.0f) {
                        vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
                    }

                    model->vertex_buffer_cache.push_back(vert);
                }
            }

            // Indices.
            if (hasIndices) {
                const tinygltf::Accessor &accessor = model->gltf2_container.accessors[primitive.indices > -1 ? primitive.indices : 0];
                const tinygltf::BufferView &bufferView = model->gltf2_container.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model->gltf2_container.buffers[bufferView.buffer];

                index_count = static_cast<uint32_t>(accessor.count);

                const void *dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const uint32_t *buf = static_cast<const uint32_t *>(dataPtr);
                    for (size_t index = 0; index < accessor.count; index++) {
                        model->index_buffer_cache.push_back(buf[index] + vertex_start);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const uint16_t *buf = static_cast<const uint16_t *>(dataPtr);
                    for (size_t index = 0; index < accessor.count; index++) {
                        model->index_buffer_cache.push_back(buf[index] + vertex_start);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const uint8_t *buf = static_cast<const uint8_t *>(dataPtr);
                    for (size_t index = 0; index < accessor.count; index++) {
                        model->index_buffer_cache.push_back(buf[index] + vertex_start);
                    }
                    break;
                }
                default: {
                    spdlog::error("Index component type {} not supported!", accessor.componentType);
                    return;
                }
                }
            }

            std::shared_ptr<Primitive> new_primitive = std::make_shared<Primitive>(
                index_start, index_count, vertex_count, primitive.material > -1 ? model->materials[primitive.material] : model->materials.back());

            new_primitive->set_bounding_box(posMin, posMax);

            newMesh->primitives.push_back(new_primitive);
        }

        // Mesh BB from BBs of primitives
        for (auto p : newMesh->primitives) {
            if (p->bb.valid && !newMesh->bb.valid) {
                newMesh->bb = p->bb;
                newMesh->bb.valid = true;
            }
            newMesh->bb.min = glm::min(newMesh->bb.min, p->bb.min);
            newMesh->bb.max = glm::max(newMesh->bb.max, p->bb.max);
        }

        newNode->mesh = newMesh;
    }

    if (parent) {
        parent->children.push_back(newNode);
    } else {
        model->nodes.push_back(newNode);
    }

    model->linear_nodes.push_back(newNode);
}

void Manager::load_skins(std::shared_ptr<Model> model) {
    spdlog::debug("ModelManager::load_skins");

    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(model);

    for (tinygltf::Skin &source : model->gltf2_container.skins) {
        std::shared_ptr<ModelSkin> newSkin = std::make_shared<ModelSkin>();

        newSkin->name = source.name;

        // Find skeleton root node.
        if (source.skeleton > -1) {
            newSkin->skeletonRoot = node_from_index(model, source.skeleton);
        }

        // Find joint nodes.
        for (int joint_index : source.joints) {
            std::shared_ptr<ModelNode> node = node_from_index(model, joint_index);

            if (node) {
                newSkin->joints.push_back(node_from_index(model, joint_index));
            }
        }

        // Get inverse bind matrices from buffer
        if (source.inverseBindMatrices > -1) {
            const tinygltf::Accessor &accessor = model->gltf2_container.accessors[source.inverseBindMatrices];
            const tinygltf::BufferView &bufferView = model->gltf2_container.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = model->gltf2_container.buffers[bufferView.buffer];

            newSkin->inverseBindMatrices.resize(accessor.count);

            std::memcpy(newSkin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
        }

        model->skins.push_back(newSkin);
    }
}

void Manager::load_textures(std::shared_ptr<Model> model) {
    spdlog::debug("loadTextures");

    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(model);

    for (tinygltf::Texture &tex : model->gltf2_container.textures) {
        tinygltf::Image image = model->gltf2_container.images[tex.source];

        TextureSampler texture_sampler;

        if (-1 == tex.sampler) {
            // No sampler specified, use a default one.
            texture_sampler.magFilter = VK_FILTER_LINEAR;
            texture_sampler.minFilter = VK_FILTER_LINEAR;
            texture_sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            texture_sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            texture_sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        } else {
            texture_sampler = model->texture_samplers[tex.sampler];
        }

        std::shared_ptr<Texture> new_texture;

        // Let's just use a random name for now.
        // TODO: Fix this!
        std::string texture_name = "glTF2 texture" + std::to_string(rand());

        // TODO: Debug!
        texture_manager->create_texture_from_glTF2_image(texture_name, image, new_texture);

        model->textures.push_back(new_texture);
    }
}

VkSamplerAddressMode Manager::get_wrap_mode(const int32_t wrapMode) {
    spdlog::debug("getVkWrapMode");

    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);

    switch (wrapMode) {
    case 10497:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case 33071:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case 33648:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }

    return VK_SAMPLER_ADDRESS_MODE_END_RANGE;
}

// TODO: std::optional!
VkFilter Manager::get_filter_mode(const int32_t filterMode) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);

    switch (filterMode) {
    case 9728:
        return VK_FILTER_NEAREST;
    case 9729:
        return VK_FILTER_LINEAR;
    case 9984:
        return VK_FILTER_NEAREST;
    case 9985:
        return VK_FILTER_NEAREST;
    case 9986:
        return VK_FILTER_LINEAR;
    case 9987:
        return VK_FILTER_LINEAR;
    }

    return VK_FILTER_MAX_ENUM;
}

void Manager::load_texture_samplers(std::shared_ptr<Model> model) {
    spdlog::debug("ModelManager::load_texture_samplers");

    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(model);

    for (tinygltf::Sampler smpl : model->gltf2_container.samplers) {
        TextureSampler sampler{};

        sampler.minFilter = get_filter_mode(smpl.minFilter);
        sampler.magFilter = get_filter_mode(smpl.magFilter);
        sampler.addressModeU = get_wrap_mode(smpl.wrapS);
        sampler.addressModeV = get_wrap_mode(smpl.wrapT);
        sampler.addressModeW = sampler.addressModeV;

        model->texture_samplers.push_back(sampler);
    }
}

void Manager::load_materials(std::shared_ptr<Model> model) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(model);

    for (tinygltf::Material &mat : model->gltf2_container.materials) {
        Material material{};

        if (mat.values.find("baseColorTexture") != mat.values.end()) {
            material.baseColorTexture = model->textures[mat.values["baseColorTexture"].TextureIndex()];
            material.texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
        }
        if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
            material.metallicRoughnessTexture = model->textures[mat.values["metallicRoughnessTexture"].TextureIndex()];
            material.texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
        }
        if (mat.values.find("roughnessFactor") != mat.values.end()) {
            material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
        }
        if (mat.values.find("metallicFactor") != mat.values.end()) {
            material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
        }
        if (mat.values.find("baseColorFactor") != mat.values.end()) {
            material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
        }
        if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
            material.normalTexture = model->textures[mat.additionalValues["normalTexture"].TextureIndex()];
            material.texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
        }
        if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
            material.emissiveTexture = model->textures[mat.additionalValues["emissiveTexture"].TextureIndex()];
            material.texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
        }
        if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
            material.occlusionTexture = model->textures[mat.additionalValues["occlusionTexture"].TextureIndex()];
            material.texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
        }
        if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
            tinygltf::Parameter param = mat.additionalValues["alphaMode"];

            if (param.string_value == "BLEND") {
                material.alphaMode = MaterialAlphaMode::ALPHAMODE_BLEND;
            }

            if (param.string_value == "MASK") {
                material.alphaCutoff = 0.5f;
                material.alphaMode = MaterialAlphaMode::ALPHAMODE_MASK;
            }
        }

        if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
            material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
        }

        if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
            material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
            material.emissiveFactor = glm::vec4(0.0f);
        }

        // Extensions.
        // @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
        if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
            auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");

            if (ext->second.Has("specularGlossinessTexture")) {
                auto index = ext->second.Get("specularGlossinessTexture").Get("index");

                material.extension.specularGlossinessTexture = model->textures[index.Get<int>()];

                auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");

                material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
                material.pbrWorkflows.specularGlossiness = true;
            }

            if (ext->second.Has("diffuseTexture")) {
                auto index = ext->second.Get("diffuseTexture").Get("index");
                material.extension.diffuseTexture = model->textures[index.Get<int>()];
            }

            if (ext->second.Has("diffuseFactor")) {
                auto factor = ext->second.Get("diffuseFactor");

                for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
                    auto val = factor.Get(i);
                    material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
                }
            }

            if (ext->second.Has("specularFactor")) {
                auto factor = ext->second.Get("specularFactor");

                for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
                    auto val = factor.Get(i);
                    material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
                }
            }
        }

        model->materials.push_back(material);
    }

    // Push a default material at the end of the list for meshes with no material assigned.
    model->materials.push_back(Material());
}

void Manager::load_animations(std::shared_ptr<Model> model) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(model);

    for (tinygltf::Animation &anim : model->gltf2_container.animations) {
        Animation animation{};
        animation.name = anim.name;

        if (anim.name.empty()) {
            animation.name = std::to_string(model->animations.size());
        }

        // Samplers.
        for (auto &samp : anim.samplers) {
            AnimationSampler sampler{};

            if (samp.interpolation == "LINEAR") {
                sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
            }

            if (samp.interpolation == "STEP") {
                sampler.interpolation = AnimationSampler::InterpolationType::STEP;
            }

            if (samp.interpolation == "CUBICSPLINE") {
                sampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;
            }

            // Read sampler input time values.
            {
                const tinygltf::Accessor &accessor = model->gltf2_container.accessors[samp.input];
                const tinygltf::BufferView &bufferView = model->gltf2_container.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model->gltf2_container.buffers[bufferView.buffer];

                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                const float *buf = static_cast<const float *>(dataPtr);

                for (size_t index = 0; index < accessor.count; index++) {
                    sampler.inputs.push_back(buf[index]);
                }

                for (auto input : sampler.inputs) {
                    if (input < animation.start) {
                        animation.start = input;
                    };
                    if (input > animation.end) {
                        animation.end = input;
                    }
                }
            }

            // Read sampler output T/R/S values
            {
                const tinygltf::Accessor &accessor = model->gltf2_container.accessors[samp.output];
                const tinygltf::BufferView &bufferView = model->gltf2_container.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model->gltf2_container.buffers[bufferView.buffer];

                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

                switch (accessor.type) {
                case TINYGLTF_TYPE_VEC3: {
                    const glm::vec3 *buf = static_cast<const glm::vec3 *>(dataPtr);

                    for (size_t index = 0; index < accessor.count; index++) {
                        sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
                    }
                    break;
                }
                case TINYGLTF_TYPE_VEC4: {
                    const glm::vec4 *buf = static_cast<const glm::vec4 *>(dataPtr);

                    for (size_t index = 0; index < accessor.count; index++) {
                        sampler.outputsVec4.push_back(buf[index]);
                    }
                    break;
                }
                default: {
                    spdlog::warn("Unknown accessor type!");
                    break;
                }
                }
            }

            animation.samplers.push_back(sampler);
        }

        // Channels
        for (auto &source : anim.channels) {
            AnimationChannel channel{};

            if (source.target_path == "rotation") {
                channel.path = AnimationChannel::PathType::ROTATION;
            }

            if (source.target_path == "translation") {
                channel.path = AnimationChannel::PathType::TRANSLATION;
            }

            if (source.target_path == "scale") {
                channel.path = AnimationChannel::PathType::SCALE;
            }

            if (source.target_path == "weights") {
                spdlog::warn("Weights not yet supported, skipping channel.");
                continue;
            }

            channel.samplerIndex = source.sampler;
            channel.node = node_from_index(model, source.target_node);

            if (!channel.node) {
                continue;
            }

            animation.channels.push_back(channel);
        }

        model->animations.push_back(animation);
    }
}

VkResult Manager::load_model_from_file(const std::string &file_name, std::shared_ptr<Model> &new_model, const float scale) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(!file_name.empty());
    assert(scale > 0.0f);

    tinygltf::TinyGLTF gltfContext;

    std::string error_message;
    std::string warning_message;

    bool is_binary_file = false;
    size_t extpos = file_name.rfind('.', file_name.length());

    if (extpos != std::string::npos) {
        // TODO: Move to tools:: find_file_extension?
        // Check if it's a binary glTF file or a raw text file.
        is_binary_file = (file_name.substr(extpos + 1, file_name.length() - extpos) == "glb");
    }

    bool fileLoaded = false;

    if (is_binary_file) {
        fileLoaded = gltfContext.LoadBinaryFromFile(&new_model->gltf2_container, &error_message, &warning_message, file_name.c_str());
    } else {
        fileLoaded = gltfContext.LoadASCIIFromFile(&new_model->gltf2_container, &error_message, &warning_message, file_name.c_str());
    }

    if (!fileLoaded) {
        spdlog::error("Could not load glTF 2.0 file: '{}'!", file_name);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (!warning_message.empty()) {
        spdlog::warn(warning_message);
    }

    if (!error_message.empty()) {
        spdlog::error(error_message);
    }

    if (fileLoaded) {
        load_texture_samplers(new_model);

        load_textures(new_model);

        load_materials(new_model);

        // TODO: scene handling with no default scene
        const tinygltf::Scene &scene =
            new_model->gltf2_container.scenes[new_model->gltf2_container.defaultScene > -1 ? new_model->gltf2_container.defaultScene : 0];

        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = new_model->gltf2_container.nodes[scene.nodes[i]];

            load_node(nullptr, node, scene.nodes[i], new_model, scale);
        }

        if (new_model->gltf2_container.animations.size() > 0) {
            load_animations(new_model);
        }

        load_skins(new_model);

        for (auto node : new_model->linear_nodes) {
            // Assign skins.
            if (node->skinIndex > -1) {
                node->skin = new_model->skins[node->skinIndex];
            }

            // Initial pose.
            if (node->mesh) {
                node->update(uniform_buffer_manager);
            }
        }
    }

    new_model->extensions = new_model->gltf2_container.extensionsUsed;

    // Create a new vertex buffer and a new index buffer for the model!
    std::size_t vertex_buffer_size = new_model->vertex_buffer_cache.size() * sizeof(ModelVertex);
    std::size_t index_buffer_size = new_model->index_buffer_cache.size() * sizeof(uint32_t);

    spdlog::debug("Vertex buffer size: {}.", vertex_buffer_size);
    spdlog::debug("Index buffer size: {}.", index_buffer_size);

    // Store how many indices are available.
    std::size_t indices_count = static_cast<uint32_t>(new_model->index_buffer_cache.size());

    spdlog::debug("glTF 2.0 model '{}' has {} indices.", file_name, indices_count);

    assert(vertex_buffer_size > 0);

    spdlog::debug("Creating a vertex buffer and an index buffer for glTF 2.0 model '{}'.", file_name);

    std::size_t number_of_vertices = new_model->vertex_buffer_cache.size();

    std::size_t number_of_indices = new_model->index_buffer_cache.size();

    // Create a vertex buffer and an index buffer for the model.
    if (number_of_indices > 0) {
        // Create a vertex buffer with an index buffer, as it should always be!
        VkResult result = mesh_buffer_manager->create_vertex_buffer_with_index_buffer(
            file_name, new_model->vertex_buffer_cache.data(), sizeof(ModelVertex), new_model->vertex_buffer_cache.size(), new_model->index_buffer_cache.data(),
            sizeof(uint32_t), number_of_indices, new_model->mesh);
        vulkan_error_check(result);
    } else {
        // Always make sure you use a model which has indices.
        // Not using an index buffer decreases performance significantly!
        VkResult result = mesh_buffer_manager->create_vertex_buffer(file_name, new_model->vertex_buffer_cache.data(), sizeof(ModelVertex),
                                                                    new_model->vertex_buffer_cache.size(), new_model->mesh);
        vulkan_error_check(result);
    }

    spdlog::debug("Calculating model dimensions.");

    get_scene_dimensions(new_model);

    return VK_SUCCESS;
}

void Manager::render_node(std::shared_ptr<ModelNode> node, VkCommandBuffer commandBuffer, VkPipelineLayout pipeline_layout, std::size_t current_image_index) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(node);
    assert(commandBuffer);

    if (node->mesh) {
        for (auto primitive : node->mesh->primitives) {
            // TODO: Implement alpha mode check here.
            auto global_descriptor_bundle = descriptor_manager->get_descriptor_bundle("inexor_global_descriptor_bundle");

            assert(global_descriptor_bundle.has_value());

            const std::vector<VkDescriptorSet> descriptor_sets = {
                global_descriptor_bundle.value()->descriptor_sets[current_image_index],
                // primitive->material.descriptorSet,
                // node->mesh->uniform_buffer->descriptor_set
            };

            // TODO: Encapsulate this by descriptor set manager?
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, static_cast<uint32_t>(descriptor_sets.size()),
                                    descriptor_sets.data(), 0, NULL);

            // TODO! PushConstantsManager
            // vkCmdPushConstants

            if (primitive->hasIndices) {
                vkCmdDrawIndexed(commandBuffer, primitive->index_count, 1, primitive->first_index, 0, 0);
            } else {
                vkCmdDraw(commandBuffer, primitive->vertex_count, 1, 0, 0);
            }
        }
    }

    for (auto &child : node->children) {
        render_node(child, commandBuffer, pipeline_layout, current_image_index);
    }
}

VkResult Manager::render_model(const std::string &internal_model_name, VkCommandBuffer commandBuffer, VkPipelineLayout pipeline_layout,
                               std::size_t current_image_index) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(!internal_model_name.empty());
    assert(commandBuffer);

    const VkDeviceSize offsets[1] = {0};

    // Check if this model exists.
    if (!does_key_exist(internal_model_name)) {
        spdlog::error("glTF 2.0 model file with internal name '{}' not found!", internal_model_name);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    auto model = get_entry(internal_model_name).value();

    // Bind the vertex and index buffer before drawing all nodes.
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->mesh->vertex_buffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, model->mesh->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    // Draw all the children nodes of the model.
    for (auto &node : model->nodes) {
        render_node(node, commandBuffer, pipeline_layout, current_image_index);
    }

    return VK_SUCCESS;
}

void Manager::calculate_bounding_box(std::shared_ptr<Model> model, std::shared_ptr<ModelNode> node, std::shared_ptr<ModelNode> parent) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(model);
    assert(node);

    BoundingBox parentBvh = parent ? parent->bvh : BoundingBox(model->dimensions.min, model->dimensions.max);

    if (node->mesh) {
        if (node->mesh->bb.valid) {
            node->aabb = node->mesh->bb.getAABB(node->getMatrix());

            if (node->children.size() == 0) {
                node->bvh.min = node->aabb.min;
                node->bvh.max = node->aabb.max;
                node->bvh.valid = true;
            }
        }
    }

    parentBvh.min = glm::min(parentBvh.min, node->bvh.min);
    parentBvh.max = glm::min(parentBvh.max, node->bvh.max);

    // Calculate the bounding boxes of the children as well!
    for (auto &child : node->children) {
        calculate_bounding_box(model, child, node);
    }
}

void Manager::get_scene_dimensions(std::shared_ptr<Model> model) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(model);

    // Calculate binary volume hierarchy for all nodes in the scene.
    for (auto node : model->linear_nodes) {
        calculate_bounding_box(model, node, nullptr);
    }

    model->dimensions.min = glm::vec3(FLT_MAX);
    model->dimensions.max = glm::vec3(-FLT_MAX);

    for (auto node : model->linear_nodes) {
        if (node->bvh.valid) {
            model->dimensions.min = glm::min(model->dimensions.min, node->bvh.min);
            model->dimensions.max = glm::max(model->dimensions.max, node->bvh.max);
        }
    }

    // Calculate scene aabb.
    model->aabb =
        glm::scale(glm::mat4(1.0f), glm::vec3(model->dimensions.max[0] - model->dimensions.min[0], model->dimensions.max[1] - model->dimensions.min[1],
                                              model->dimensions.max[2] - model->dimensions.min[2]));

    model->aabb[3][0] = model->dimensions.min[0];
    model->aabb[3][1] = model->dimensions.min[1];
    model->aabb[3][2] = model->dimensions.min[2];
}

void Manager::update_animation(std::shared_ptr<Model> model, const uint32_t index, const float time) {
    assert(model_manager_initialised);
    assert(texture_manager);
    assert(uniform_buffer_manager);
    assert(mesh_buffer_manager);
    assert(model);

    if (model->animations.empty()) {
        spdlog::warn("glTF 2.0 Model file '{}' does not contain animations!", model->name);
        return;
    }

    if (index > static_cast<uint32_t>(model->animations.size()) - 1) {
        spdlog::error("glTF 2.0 Model file '{}': No animation with index {}.", model->name, index);
        return;
    }

    Animation &animation = model->animations[index];

    bool updated = false;

    for (auto &channel : animation.channels) {
        AnimationSampler &sampler = animation.samplers[channel.samplerIndex];

        if (sampler.inputs.size() > sampler.outputsVec4.size()) {
            continue;
        }

        for (std::size_t i = 0; i < sampler.inputs.size() - 1; i++) {
            if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1])) {
                float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);

                if (u <= 1.0f) {
                    switch (channel.path) {
                    case AnimationChannel::PathType::TRANSLATION: {
                        glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
                        channel.node->translation = glm::vec3(trans);
                        break;
                    }
                    case AnimationChannel::PathType::SCALE: {
                        glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
                        channel.node->scale = glm::vec3(trans);
                        break;
                    }
                    case AnimationChannel::PathType::ROTATION: {
                        glm::quat q1;
                        q1.x = sampler.outputsVec4[i].x;
                        q1.y = sampler.outputsVec4[i].y;
                        q1.z = sampler.outputsVec4[i].z;
                        q1.w = sampler.outputsVec4[i].w;

                        glm::quat q2;
                        q2.x = sampler.outputsVec4[i + 1].x;
                        q2.y = sampler.outputsVec4[i + 1].y;
                        q2.z = sampler.outputsVec4[i + 1].z;
                        q2.w = sampler.outputsVec4[i + 1].w;

                        channel.node->rotation = glm::normalize(glm::slerp(q1, q2, u));
                        break;
                    }
                    }
                    updated = true;
                }
            }
        }
    }

    if (updated) {
        for (auto &node : model->nodes) {
            node->update(uniform_buffer_manager);
        }
    }
}

VkResult Manager::load_model_from_glTF2_file(const std::string &internal_model_name, const std::string &glTF2_file_name) {
    assert(model_manager_initialised);

    if (does_key_exist(internal_model_name)) {
        spdlog::error("A glTF 2.0 model with internal name '{}' does already exist!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    std::shared_ptr<Model> new_model = std::make_shared<Model>();

    VkResult result = load_model_from_file(glTF2_file_name, new_model);
    vulkan_error_check(result);

    add_entry(internal_model_name, new_model);

    return VK_SUCCESS;
}

std::shared_ptr<ModelNode> Manager::find_node(std::shared_ptr<ModelNode> parent, const uint32_t index) {
    assert(model_manager_initialised);
    assert(parent);

    spdlog::debug("Finding node by id {}.", index);

    std::shared_ptr<ModelNode> node_found = nullptr;

    if (parent->index == index) {
        return parent;
    }

    for (auto &child : parent->children) {
        node_found = find_node(child, index);

        if (node_found) {
            break;
        }
    }
    return node_found;
}

VkResult Manager::render_all_models(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, std::size_t current_image_index) {
    assert(model_manager_initialised);

    const auto models = get_all_values();

    for (const auto &model : models) {
        VkDeviceSize offsets[1] = {0};

        vkCmdBindVertexBuffers(command_buffer, 0, 1, &model->mesh->vertex_buffer.buffer, offsets);

        // Only bind index buffer if available.
        if (model->mesh->index_buffer_available) {
            vkCmdBindIndexBuffer(command_buffer, model->mesh->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        }

        // Opaque primitives first.
        for (auto node : model->nodes) {
            render_node(node, command_buffer, pipeline_layout, current_image_index);
        }

        // Alpha masked primitives.
        for (auto node : model->nodes) {
            render_node(node, command_buffer, pipeline_layout, current_image_index);
        }

        // TODO: PBR alpha blend here!
    }

    return VK_SUCCESS;
}

VkResult Manager::create_model_descriptors(const std::size_t number_of_images_in_swapchain) { return VK_SUCCESS; }

VkResult Manager::setup_node_descriptor_set(std::shared_ptr<ModelNode> node) {
    if (node->mesh) {
        // TODO!
        /*
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};

        descriptorSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool     = descriptor_pool;
        descriptorSetAllocInfo.pSetLayouts        = &descriptor_set_for_gltf_model_nodes;
        descriptorSetAllocInfo.descriptorSetCount = 1;

        VkResult result = vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &node->mesh->uniform_buffer->descriptor_set);
        vulkan_error_check(result);

        VkWriteDescriptorSet writeDescriptorSet{};

        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstSet = node->mesh->uniform_buffer->descriptor_set;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.pBufferInfo = &node->mesh->uniform_buffer->descriptor_buffer_info;

        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
        */
    }

    for (auto &child : node->children) {
        setup_node_descriptor_set(child);
    }

    return VK_SUCCESS;
}

std::size_t Manager::get_model_count() {
    assert(model_manager_initialised);

    return get_entry_count();
}

std::shared_ptr<ModelNode> Manager::node_from_index(std::shared_ptr<Model> model, const uint32_t index) {
    assert(model_manager_initialised);
    assert(model);

    spdlog::debug("Looking up node from index for model '{}' index {}.", model->name, index);

    std::shared_ptr<ModelNode> node_found = nullptr;

    for (auto &node : model->nodes) {
        node_found = find_node(node, index);

        if (node_found) {
            break;
        }
    }
    return node_found;
}

} // namespace inexor::vulkan_renderer::gltf_model
