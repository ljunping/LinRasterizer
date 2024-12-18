//
// Created by Lin on 2024/12/4.
//

#ifndef TYPE_H
#define TYPE_H

#define CREATE_SHARE_OBJECT_BY_TYPE(Type,...) (TypeFactory::create_share_type_inst<Type>(__VA_ARGS__))
#define CREATE_OBJECT_BY_TYPE(Type,...) (TypeFactory::create_type_inst<Type>(__VA_ARGS__))
#define CREATE_OBJECT(type_id) (TypeFactory::create_type_inst_by_id(type_id))
#define GET_OBJECT(inst_id) ((TypeFactory::get_type_inst_by_inst_id(inst_id)))
#define GET_TYPE_OBJECT(type,inst_id) ((type*)(TypeFactory::get_type_inst_by_inst_id(inst_id)))
#define DESTROY_OBJECT(inst) (TypeFactory::destroy_type_inst(inst))
#define IS_SUBCLASS(parent_id,subclass_id) (TypeFactory::subclass(parent_id,subclass_id))
#define PARENT_CLASS(type_id) (TypeFactory::get_parent_type(type_id))


#define INIT_TYPE(type_name,base_type)\
public:\
    using parent_class = base_type;\
    friend class TypeFactory;\
    static int get_type_id(){return type_id;};\
    int inst_get_type_id() override {return type_id;};  \
private:\
    static int type_id;\
    static int register_type(){return TypeFactory::register_type<type_name>();};\
    static Object* create()\
    {\
        auto obj=new type_name();\
        return obj;\
    };\
protected:\
type_name():base_type(){};\

#define REGISTER_TYPE(type_name)\
type_name::type_id = TypeFactory::register_type<type_name>();\


#define DEFINE_TYPE(type_name)\
int type_name::type_id = 0;

#include <unordered_map>

#include "CommonMacro.h"
#include "VectorRemoveEasy.h"


class Object
{
    friend class TypeFactory;
private:
    static int type_id;
    int instance_id;
    static int register_type()
    {
        return 0;
    }
public:
    static int get_type_id(){return type_id;};
    int get_instance_id() const {return instance_id;};
    virtual int inst_get_type_id(){return type_id;};\
    static Object* create_Object()\
    {
        return new Object();
    }
    Object() = default;
    virtual ~Object() = default;
    virtual void on_create(){};
    virtual void on_delete(){};
};

class TypeFactory
{
    static int type_id;
    static std::unordered_map<int, Object* (*)()> type_creator_map;
    static std::unordered_map<int, int> parent_map;
    static int objet_inst_id;
    static std::unordered_map<int, Object*> objet_inst_map;
public:
    template<class T>
    static int register_type();
    template <class T, class ... Args>
    static T* create_type_inst(Args&&... args);
    template <class T, class ... Args>
    static std::shared_ptr<T> create_share_type_inst(Args&&... args);

    static int get_parent_type(int type_id);

    static bool subclass(int parent_id,int child_type_id);

    static Object* create_type_inst_by_id(int type_id);

    static Object* get_type_inst_by_inst_id(int inst_id);

    static void destroy_type_inst(Object* obj)
    {
        objet_inst_map.erase(obj->instance_id);
        obj->on_delete();
        delete obj;
    }
    static void RegisterTypes();
};
template <class T, class ... Args>
T* TypeFactory::create_type_inst(Args&&... args)
{
    auto res = new T(std::forward<Args>(args)...);
    res->instance_id = ++objet_inst_id;
    objet_inst_map[res->instance_id] =(Object*) res;
    res->on_create();
    return res;
}

template <class T, class ... Args>
SHARE_PTR<T> TypeFactory::create_share_type_inst(Args&&... args)
{
    auto obj = create_type_inst<T>(std::forward<Args>(args)...);
    return SHARE_PTR<T>(obj,TypeFactory::destroy_type_inst);
}

template <class T>
int TypeFactory::register_type()
{
    static int static_type_id = 0;
    if (static_type_id > 0)
    {
        return static_type_id;
    }
    ++type_id;
    if constexpr (!std::is_same<T, Object>::value)
    {
        T::parent_class::type_id = T::parent_class::register_type();
        parent_map[type_id] = T::parent_class::type_id;
    }
    type_creator_map[type_id] = T::create;
    static_type_id = type_id;
    return type_id;
}

template <typename T>
class ObjectManger
{
    static_assert(std::is_base_of<Object, T>::value, "T must inherit from Object");
protected:
    VectorRemoveEasy<T*> objects;
public:
    virtual ~ObjectManger() = default;
    virtual void on_create_obj(T* obj);
    virtual void on_delete_obj(T* obj);
};

template <typename T>
void ObjectManger<T>::on_create_obj(T* obj)
{
    objects.push_back(obj);
}

template <typename T>
void ObjectManger<T>::on_delete_obj(T* obj)
{
    objects.easy_remove(obj);
}


#endif //TYPE_H
