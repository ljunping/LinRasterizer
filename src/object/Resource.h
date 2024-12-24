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
public:

    template <class T, class ... Args>
    static T* create_resource(const char* name, Args&&... args);
    template <class T, class ... Args>
    static T* create_unnamed_resource(Args&&... args);
    template <class T, class ... Args>
    static T* get_or_create_resource(const char* name, Args&&... args);

    static void destroy_resource(const char* name);

    static void destroy_resource(int resource_id);

    template<class  T>
    static T* get_resource(int id);
    template <class T>
    static T* get_resource(const char* name);
    virtual int get_resource_id() const;
};


template <class T, class ... Args>
T* Resource::create_resource(const char* name, Args&&... args)
{
    static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");
    RUNTIME_ASSERT(!resource_name2_id.contains(name), std::format("Resource with name {} already exists", name));
    auto type_inst = CREATE_OBJECT_BY_TYPE(T, std::forward<Args>(args)...);
    resource_name2_id[name] = type_inst->get_resource_id();
    resource_id2_name[type_inst->get_resource_id()] = name;
    return type_inst;
}

template <class T, class ... Args>
T* Resource::create_unnamed_resource( Args&&... args)
{
    return CREATE_OBJECT_BY_TYPE(T, std::forward<Args>(args)...);
}

template <class T, class ... Args>
T* Resource::get_or_create_resource(const char* name, Args&&... args)
{
    auto res = get_resource<T>(name);
    if (!res)
    {
        res = create_resource<T>(name, std::forward<Args>(args)...);
    }
    return res;
}

template <class T>
T* Resource::get_resource(int id)
{
    return GET_TYPE_OBJECT(T, id);
}

template <class T>
T* Resource::get_resource(const char* name)
{
    auto iter = resource_name2_id.find(name);
    if (resource_name2_id.end() != iter)
    {
        return get_resource<T>(resource_name2_id[name]);
    }
    return nullptr;
}
#endif //RESOURCE_H
