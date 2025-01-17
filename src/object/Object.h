//
// Created by Lin on 2024/12/4.
//

#ifndef TYPE_H
#define TYPE_H

#define CREATE_SHARE_OBJECT_BY_TYPE(Type,...) (TypeFactory::create_share_type_inst<Type>(__VA_ARGS__))
#define CREATE_OBJECT_BY_TYPE(Type,...) (TypeFactory::create_type_inst<Type>(__VA_ARGS__))
#define CREATE_OBJECT(type_id) (TypeFactory::create_type_inst_by_id(type_id))
#define COPY_OBJECT(obj) (TypeFactory::copy_object(obj))

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
    void clone(Object* dst) override { (*( type_name*)(dst))=(*this); }\
protected:\
    type_name():base_type(){};\
    type_name(const type_name&) = default;\
    type_name(type_name&&) = default;\
    type_name& operator=(const type_name&) = default;\
    type_name& operator=(type_name&&) = default;\

#if DEBUG
#define REGISTER_TYPE(type_name)\
type_name::type_id = TypeFactory::register_type<type_name>();\
// debug_objects[type_name::type_id]=CREATE_OBJECT(type_name::type_id);
#else

#define REGISTER_TYPE(type_name)\
type_name::type_id = TypeFactory::register_type<type_name>();\

#endif


#define DEFINE_TYPE(type_name)\
int type_name::type_id = 0;

#include <unordered_map>
#include "CommonMacro.h"
#include "debug.h"
#include "VectorRemoveEasy.h"

class Object
{
    friend class TypeFactory;
private:
    static int type_id;
    int instance_id;
    static int register_type();
    virtual void clone(Object* dst)
    {
        (*dst) = *this;
    }
protected:
    Object(const Object&) = default;
    Object(Object&&) = default;
    Object& operator=(const Object&) = default;
    Object& operator=(Object&&) = default;
public:
    static int get_type_id();
    int get_instance_id() const;
    virtual int inst_get_type_id();
    static Object* create();
    Object() = default;
    virtual ~Object() = default;
    virtual void on_create();
    virtual void on_delete();
};

class TypeFactory
{
    static int type_id;
    static std::unordered_map<int, Object* (*)()> type_creator_map;
    static std::unordered_map<int, int> parent_map;
    static int objet_inst_id;
    static std::unordered_map<int, Object*> objet_inst_map;
    static std::unordered_map<int, WEAK_PTR<Object>> weak_ptr_objet_inst_map;
public:
    template<class T>
    static int register_type();
    template <class T, class ... Args>
    static T* create_type_inst(Args&&... args);
    template <class T,class Dealloc, class ... Args>
    static std::shared_ptr<T> create_custom_share_type_inst(Dealloc dealloc, Args&&... args);

    template <class T, class ... Args>
    static std::shared_ptr<T> create_share_type_inst(Args&&... args);

    static int get_parent_type(int type_id);
    template<class T>
    static T* copy_object(T* obj);

    template<class T,class Dealloc>
    static SHARE_PTR<T> copy_share_ptr_object(T* obj, Dealloc dealloc);

    static bool subclass(int parent_id,int child_type_id);

    static Object* create_type_inst_by_id(int type_id);

    static Object* get_type_inst_by_inst_id(int inst_id);

    static SHARE_PTR<Object> get_shared_ptr_by_inst_id(int inst_id);


    static void destroy_type_inst(Object* obj)
    {
        objet_inst_map.erase(obj->instance_id);
        auto ptr = weak_ptr_objet_inst_map.find(obj->instance_id);
        if (ptr != weak_ptr_objet_inst_map.end())
        {
            if (!ptr->second.expired())
            {
                throw std::runtime_error("Object is not expired");
            }
        }
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
    objet_inst_map[res->instance_id] =static_cast<Object*>(res);
    res->on_create();
    return res;
}

template <class T,class Dealloc, class ... Args>
SHARE_PTR<T> TypeFactory::create_custom_share_type_inst(Dealloc dealloc, Args&&... args)
{
    auto obj = create_type_inst<T>(std::forward<Args>(args)...);
    auto shared_ptr = SHARE_PTR<T>(obj, dealloc);
    weak_ptr_objet_inst_map[obj->instance_id] = shared_ptr;
    return shared_ptr;
}

template <class T, class ... Args>
std::shared_ptr<T> TypeFactory::create_share_type_inst(Args&&... args)
{
    auto obj = create_type_inst<T>(std::forward<Args>(args)...);
    auto shared_ptr = SHARE_PTR<T>(obj, destroy_type_inst);
    weak_ptr_objet_inst_map[obj->instance_id] = shared_ptr;
    return shared_ptr;
}

template <class T>
T* TypeFactory::copy_object(T* obj)
{
    Object* inst = CREATE_OBJECT(obj->inst_get_type_id());
    auto instance_id = inst->instance_id;
    obj->clone(inst);
    inst->instance_id = instance_id;
    return dynamic_cast<T*>(inst);
}

template <class T, class Dealloc>
std::shared_ptr<T> TypeFactory::copy_share_ptr_object(T* obj, Dealloc dealloc)
{
    SHARE_PTR<Object> inst;
    if(dealloc)
    {
        inst = SHARE_PTR<T>(copy_object(obj), dealloc);
        weak_ptr_objet_inst_map[inst->instance_id] = inst;
    }else
    {
        inst = SHARE_PTR<T>(copy_object(obj), destroy_type_inst);
        weak_ptr_objet_inst_map[inst->instance_id] = inst;
    }
    return std::dynamic_pointer_cast<T>(inst);
}




template <class T>
int TypeFactory::register_type()
{
    static int static_type_id = 0;
    if (static_type_id > 0)
    {
        return static_type_id;
    }
    int _typeid = ++type_id;
    if constexpr (!std::is_same<T, Object>::value)
    {
        T::parent_class::type_id = T::parent_class::register_type();
        parent_map[_typeid] = T::parent_class::type_id;
    }
    type_creator_map[_typeid] = T::create;
    static_type_id = _typeid;
    return _typeid;
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
    VectorRemoveEasy<T*>& get_objects() { return objects; }
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
