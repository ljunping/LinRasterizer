//
// Created by Lin on 2024/12/17.
//

#ifndef RESOURCE_H
#define RESOURCE_H
#include "debug.h"
#include "Object.h"


class Resource : public Object
{
    INIT_TYPE(Resource, Object)
    static std::unordered_map<std::string, int> resource_name2_id;
    static std::unordered_map<int, std::string> resource_id2_name;
    static void destroy_resource(Resource* resource);
public:

    template <class T, class ... Args>
    static SHARE_PTR<T> create_resource(const char* name, Args&&... args);

    template <class T, class ... Args>
    static SHARE_PTR<T> create_default_resource(Args&&... args);

    template <class T, class ... Args>
    static SHARE_PTR<T> create_unnamed_resource(Args&&... args);
    template <class T, class ... Args>
    static SHARE_PTR<T> get_or_create_resource(const char* name, Args&&... args);

    template <class T, class ... Args>
    static SHARE_PTR<T> get_or_create_default_resource(Args&&... args);

    template<class  T>
    static SHARE_PTR<T> copy_resource(const SHARE_PTR<T>& resource);

    template<class  T>
    static SHARE_PTR<T> get_resource(int id);
    template <class T>
    static SHARE_PTR<T> get_resource(const char* name);
    template <class T>
    static std::string str_name(const char* name);
    template <class T>
    static  std::string default_str_name();
    virtual int get_resource_id() const;
};


template <class T, class ... Args>
SHARE_PTR<T> Resource::create_resource(const char* _name, Args&&... args)
{
    static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");
    std::string name = str_name<T>(_name);
    static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");
    RUNTIME_ASSERT(!resource_name2_id.contains(name), std::format("Resource with name {} already exists", name));
    auto type_inst = TypeFactory::create_custom_share_type_inst<T>(destroy_resource, std::forward<Args>(args)...);
    resource_name2_id[name] = type_inst->get_resource_id();
    resource_id2_name[type_inst->get_resource_id()] = name;
    return type_inst;
}

template <class T, class ... Args>
std::shared_ptr<T> Resource::create_default_resource(Args&&... args)
{
    return create_resource<T>("default", std::forward<Args>(args)...);
}

template <class T, class ... Args>
SHARE_PTR<T> Resource::create_unnamed_resource(Args&&... args)
{
    return TypeFactory::create_custom_share_type_inst<T>(destroy_resource, std::forward<Args>(args)...);
}

template <class T, class ... Args>
SHARE_PTR<T> Resource::get_or_create_resource(const char* name, Args&&... args)
{
    auto res = get_resource<T>(name);
    if (!res)
    {
        res = create_resource<T>(name, std::forward<Args>(args)...);
    }
    return res;
}

template <class T, class ... Args>
std::shared_ptr<T> Resource::get_or_create_default_resource(Args&&... args)
{
    return get_or_create_resource<T>("default", std::forward<Args>(args)...);
}

template <class T>
std::shared_ptr<T> Resource::copy_resource(const std::shared_ptr<T>& resource)
{
    return TypeFactory::copy_share_ptr_object(resource.get(), destroy_resource);
}

template <class T>
SHARE_PTR<T> Resource::get_resource(int id)
{
    return std::dynamic_pointer_cast<T>(TypeFactory::get_shared_ptr_by_inst_id(id));
}

template <class T>
SHARE_PTR<T> Resource::get_resource(const char* _name)
{
    std::string name = str_name<T>(_name);
    auto iter = resource_name2_id.find(name);
    if (resource_name2_id.end() != iter)
    {
        return get_resource<T>(resource_name2_id[name]);
    }
    return nullptr;
}


template <class T>
std::string Resource::str_name(const char* name)
{
    std::string name_str = name;
    name_str += "Type_Id=" + std::to_string(T::get_type_id());
    return name_str;
}

template <class T>
std::string Resource::default_str_name()
{
    return str_name<T>("default");
}


#endif //RESOURCE_H
