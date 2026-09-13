#include "instance.h"
#include "../mem/memory.h"
#include "offsets.hpp"


std::string rbx::nameable_t::get_string_value()
{
    if (!address)
        return "unknown";

    std::string result;

    for (int i = 0; i < 8; ++i)
    {
        const char c =
            memory->read<char>(
                address + Offsets::Misc::Value + i
            );

        if (c == '\0')
            break;

        result += c;
    }

    return result;
}


std::string rbx::nameable_t::get_name()
{
    if (!address)
        return "unknown";

    const std::uint64_t name_ptr =
        memory->read<std::uint64_t>(
            address + Offsets::Instance::Name
        );

    if (!name_ptr)
        return "unknown";

    return memory->read_string(name_ptr);
}


std::string rbx::nameable_t::get_class_name()
{
    if (!address)
        return "unknown";

    const std::uint64_t class_descriptor =
        memory->read<std::uint64_t>(
            address + Offsets::Instance::ClassDescriptor
        );

    if (!class_descriptor)
        return "unknown";

    const std::uint64_t class_name =
        memory->read<std::uint64_t>(
            class_descriptor + Offsets::Instance::ClassName
        );

    if (!class_name)
        return "unknown";

    return memory->read_string(class_name);
}


std::vector<rbx::instance_t> rbx::interface_t::get_children()
{
    std::vector<rbx::instance_t> children;

    if (!address)
        return children;

    const std::uint64_t children_start =
        memory->read<std::uint64_t>(
            address + Offsets::Instance::ChildrenStart
        );

    if (!children_start)
        return children;

    const std::uint64_t children_end =
        memory->read<std::uint64_t>(
            children_start + Offsets::Instance::ChildrenEnd
        );

    if (!children_end || children_end <= children_start)
        return children;

    for (
        std::uint64_t current = memory->read<std::uint64_t>(children_start);
        current < children_end;
        current += sizeof(std::shared_ptr<void*>)
    )
    {
        const std::uint64_t child =
            memory->read<std::uint64_t>(current);

        if (child)
            children.emplace_back(child);
    }

    return children;
}


rbx::instance_t rbx::interface_t::find_first_child(
    std::string_view name)
{
    for (rbx::instance_t& child : get_children())
    {
        if (child.get_name() == name)
            return child;
    }

    return {};
}


rbx::instance_t rbx::interface_t::find_first_child_of_class(
    std::string_view class_name)
{
    for (rbx::instance_t& child : get_children())
    {
        if (child.get_class_name() == class_name)
            return child;
    }

    return {};
}


rbx::instance_t resolve(
    rbx::instance_t root,
    const std::vector<std::string>& path)
{
    rbx::instance_t current = root;

    for (const std::string& name : path)
    {
        if (!current.address)
            return {};

        current = current.find_first_child(name);

        if (!current.address)
            return {};
    }

    return current;
}
