#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "../mem/memory.h"
#include "offsets.hpp"

namespace rbx
{
	class instance_t;

	struct addressable_t
	{
		std::uint64_t address;

		addressable_t() : address(0) {}
		addressable_t(std::uint64_t address) : address(address) {}
	};

	struct nameable_t : public addressable_t
	{
		using addressable_t::addressable_t;

		template <typename T>
		T get_value_as()
		{
			return memory->read<T>(this->address + Offsets::Misc::Value);
		}
		std::string get_string_value();
		std::string get_name();
		std::string get_class_name();
		//std::string get_attribute(std::string_view name_to_search);
		template<typename T>
		T get_attribute(std::string_view name_to_search)
		{
			uint64_t AttributeList1 = memory->read<uint64_t>(this->address + Offsets::Instance::AttributeContainer);
			uint64_t AttributeList = memory->read<uint64_t>(AttributeList1 + Offsets::Instance::AttributeList);

			for (int i = 0x0; i < 0x1000; i += Offsets::Instance::AttributeToNext)
			{
				std::string Name = memory->read_string(memory->read<uint64_t>(AttributeList + i));

				if (Name == name_to_search)
				{
					return memory->read<T>(AttributeList + i + Offsets::Instance::AttributeToValue);
				}
			}

			return T{};
		}
	};

	struct interface_t
	{
		template <typename T>
		std::vector<T> get_children();

		std::vector<rbx::instance_t> get_children();
		rbx::instance_t find_first_child(std::string_view str);
		rbx::instance_t find_first_child_of_class(std::string_view str);
	};

	struct instance_t : public nameable_t, public interface_t
	{
		using nameable_t::nameable_t;
	};
}

template <typename T>
std::vector<T> rbx::interface_t::get_children()
{
	rbx::instance_t* base = static_cast<rbx::instance_t*>(this);

	std::uint64_t start = memory->read<uint64_t>(base->address + Offsets::Instance::ChildrenStart);
	std::uint64_t end = memory->read<uint64_t>(start + Offsets::Instance::ChildrenEnd);

	std::vector<T> children;

	for (std::uint64_t instance = memory->read<std::uint64_t>(start); instance < end; instance += sizeof(std::shared_ptr<void*>))
	{
		children.emplace_back(memory->read<uint64_t>(instance));
	}
	return children;
}

template<>
inline std::string rbx::nameable_t::get_attribute<std::string>(std::string_view name_to_search)
{
	uint64_t AttributeList1 = memory->read<uint64_t>(this->address + Offsets::Instance::AttributeContainer);
	uint64_t AttributeList = memory->read<uint64_t>(AttributeList1 + Offsets::Instance::AttributeList);

	for (int i = 0x0; i < 0x1000; i += Offsets::Instance::AttributeToNext)
	{
		std::string Name = memory->read_string(memory->read<uint64_t>(AttributeList + i));

		if (Name == name_to_search)
		{
			return memory->read_string(AttributeList + i + Offsets::Instance::AttributeToValue);
		}
	}

	return "";
}

rbx::instance_t resolve(rbx::instance_t root, const std::vector<std::string>& path);