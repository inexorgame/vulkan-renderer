#pragma once

#include "inexor_bounding_box.hpp"
#include "inexor_gltf_model_mesh.hpp"
#include "../uniform-buffer/vk_uniform_buffer.hpp"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


#include <string>
#include <memory>
#include <vector>
#include <cassert>


namespace inexor {
namespace vulkan_renderer {
namespace gltf2 {


	struct InexorModelNode;

	/// 
	struct InexorModelSkin
	{
		std::string name;

		std::shared_ptr<InexorModelNode> skeletonRoot = nullptr;
		
		std::vector<glm::mat4> inverseBindMatrices;
		
		std::vector<std::shared_ptr<InexorModelNode>> joints;
	};


	/// 
	struct InexorModelNode
	{
		std::shared_ptr<InexorModelNode> parent;
		
		uint32_t index;
		
		std::vector<std::shared_ptr<InexorModelNode>> children;
		
		glm::mat4 matrix;
		
		std::string name;
		
		std::shared_ptr<InexorModelMesh> mesh;
		
		std::shared_ptr<InexorModelSkin> skin;

		int32_t skinIndex = -1;
		
		glm::vec3 translation{};
		
		glm::vec3 scale{ 1.0f };
		
		glm::quat rotation{};
		
		BoundingBox bvh;
		
		BoundingBox aabb;


		glm::mat4 localMatrix()
		{
			return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
		}


		glm::mat4 getMatrix()
		{
			glm::mat4 m = localMatrix();

			std::shared_ptr<InexorModelNode> p = parent;

			while(p)
			{
				m = p->localMatrix() * m;
				p = p->parent;
			}

			return m;
		}


		void update(const std::shared_ptr<VulkanUniformBufferManager> uniform_buffer_manager)
		{
			if(mesh)
			{
				glm::mat4 m = getMatrix();

				if(skin)
				{
					mesh->uniform_block.matrix = m;
					
					// Update join matrices.
					glm::mat4 inverseTransform = glm::inverse(m);
					
					size_t numJoints = std::min((uint32_t)skin->joints.size(), MAX_NUM_JOINTS);
					
					for(size_t i = 0; i < numJoints; i++)
					{
						std::shared_ptr<InexorModelNode> jointNode = skin->joints[i];
						
						glm::mat4 jointMat = jointNode->getMatrix() * skin->inverseBindMatrices[i];
						jointMat = inverseTransform * jointMat;
						mesh->uniform_block.joint_matrix[i] = jointMat;
					}

					mesh->uniform_block.joint_count = (float)numJoints;

					spdlog::debug("Updating uniform buffers.");

					// TODO: Call uniform buffer manager?
					//std::memcpy(mesh->uniform_buffer->allocation_info.pMappedData, &mesh->uniform_block, sizeof(glm::mat4));
				}
				else
				{
					spdlog::debug("Updating uniform buffers.");

					// TODO: Call uniform buffer manager?
					//std::memcpy(mesh->uniform_buffer->allocation_info.pMappedData, &m, sizeof(glm::mat4));
				}
			}

			for(auto& child : children)
			{
				child->update(uniform_buffer_manager);
			}
		}


		~InexorModelNode()
		{
		}
	};

};
};
};
