#pragma once

#include <unordered_map>
#include <memory_resource>

namespace HGEGraphics
{
	template<typename ResourceDescriptor, typename ResourceType, bool neverRelease>
	class ResourcePool
	{
	public:
		ResourcePool(ResourcePool<ResourceDescriptor, ResourceType, neverRelease>* upstream, std::pmr::memory_resource* const memory_resource)
			: m_upstream(upstream), m_resources(memory_resource)
		{
		}

		void destroy()
		{
			if (m_upstream)
			{
				for (auto& [key, resource] : m_resources)
				{
					m_upstream->releaseResource(resource);
				}
			}
			else
			{
				for (auto& [key, resource] : m_resources)
				{
					destroyResource_impl(resource);
				}
			}
		}

		virtual ~ResourcePool()
		{
			if (!m_upstream)
			{
				for (auto& [key, resource] : m_resources)
				{
					delete resource;
				}
			}
			m_resources.clear();
		}

		ResourceType* getResource(const ResourceDescriptor& descriptor)
		{
			auto iter = m_resources.find(descriptor);
			if (iter != m_resources.end())
			{
				auto resource = iter->second;
				if (!neverRelease)
					m_resources.erase(iter);
				return resource;
			}

			if (m_upstream)
				return m_upstream->getResource(descriptor);
			else
			{
				auto res = getResource_impl(descriptor);
				if (neverRelease)
					m_resources.insert({ descriptor, res });
				return res;
			}
		}
		void releaseResource(ResourceType* resource)
		{
			if constexpr (!neverRelease)
				m_resources.insert({ resource->descriptor(), resource });
		}

		ResourcePool<ResourceDescriptor, ResourceType, neverRelease>* upstream() const { return m_upstream; }

	protected:
		virtual ResourceType* getResource_impl(const ResourceDescriptor& descriptor) = 0;
		virtual void destroyResource_impl(ResourceType* resource) = 0;

	protected:
		std::pmr::unordered_multimap<ResourceDescriptor, ResourceType*> m_resources;
		ResourcePool<ResourceDescriptor, ResourceType, neverRelease>* m_upstream = nullptr;
	};
}
