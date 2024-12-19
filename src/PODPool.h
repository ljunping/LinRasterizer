//
// Created by Lin on 2024/11/19.
//

#ifndef POOL_H
#define POOL_H
#include <mutex>
#include <queue>
#include<unordered_map>
#define POOL_MAX_SIZE 1024
template<class T>
class PODPool
{
public:
    std::queue<T*> POOL_QUEUE;
    std::mutex mtx;
    static PODPool POOL;
    T* create();
    void recycle(T* obj);
    void clear();
    int size();
};

template<class T,int Flag>
class PODArrayPool
{
public:
    std::unordered_map<int, std::queue<T*>> POOL_MAP;
    std::mutex mtx;
    static PODArrayPool POOL;
    T* create(int size);
    void recycle(T* obj,int size);
    void clear(int type);
    void clear();
    int size();
    bool contain_type(int type);
};


#include <concepts>
#include <type_traits>

// 定义概念：要求 T 有一个成员函数 doSomething() 和一个类型别名 value_type
template<typename T>
concept ResetInterface = requires(T t) {
    { t.reset() } -> std::same_as<void>;  // 必须有 doSomething()，且返回类型为 void
};

template<ResetInterface T>
class Pool
{
public:
    std::queue<T*> POOL_QUEUE;
    std::mutex mtx;
    static Pool POOL;
    T* create();
    void recycle(T* obj);
    void clear();
    int size();
};

template<ResetInterface T,int Flag>
class ArrayPool
{
public:
    std::unordered_map<int, std::queue<T*>> POOL_MAP;
    std::mutex mtx;
    static ArrayPool POOL;
    T* create(int size);
    void recycle(T* obj,int size);
    void clear();
    void clear(int type);
    int size();
    bool contain_type(int type);
};

template <class T>
T* PODPool<T>::create()
{
    if constexpr (!std::is_pod<T>::value)
    {
        throw("PODPool does not have a pod type");
    }
    std::lock_guard<std::mutex> lock(mtx);
    if (POOL_QUEUE.empty())
    {
        POOL_QUEUE.emplace(new T());
    }
    auto front = POOL_QUEUE.front();
    POOL_QUEUE.pop();
    return front;
}


template <class T>
void PODPool<T>::recycle(T* obj)
{
    if constexpr (!std::is_pod<T>::value)
    {
        throw("PODPool does not have a pod type");
    }
    std::lock_guard<std::mutex> lock(mtx);
    memset(obj, 0, sizeof(T));
    if (POOL_QUEUE.size() >= POOL_MAX_SIZE)
    {
        delete obj;
    }
    else
    {
        POOL_QUEUE.emplace(obj);
    }
}

template <class T>
void PODPool<T>::clear()
{
    while (POOL_QUEUE.size() > 0)
    {
        delete POOL_QUEUE.front();
        POOL_QUEUE.pop();
    }
}

template <class T>
int PODPool<T>::size()
{
    return POOL_QUEUE.size();
}


template <class T,int Flag>
T* PODArrayPool<T,Flag>::create(int size)
{
    if constexpr (!std::is_pod<T>::value)
    {
        throw("PODArrayPool does not have a pod type");
    }
    std::lock_guard<std::mutex> lock(mtx);

    if (!POOL_MAP.contains(size))
    {
        POOL_MAP[size] = std::queue<T*>();
    }
    auto& queue = POOL_MAP[size];
    if (queue.empty())
    {
        queue.emplace(new T[size]);
    }
    auto front = queue.front();
    queue.pop();
    return front;
}


template <class T,int Flag>
void PODArrayPool<T,Flag>::recycle(T* obj,int size)
{
    if constexpr (!std::is_pod<T>::value)
    {
        throw("PODArrayPool does not have a pod type");
    }
    std::lock_guard<std::mutex> lock(mtx);
    if (!POOL_MAP.contains(size))
    {
        return;
    }
    memset(obj, 0, sizeof(T) * size);
    auto& queue = POOL_MAP[size];
    if (queue.size() >= 10)
    {
        delete[] obj;
    }
    else
    {
        queue.push(obj);
    }
}

template <class T,int Flag>
void PODArrayPool<T,Flag>::clear(int type)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (POOL_MAP.contains(type))
    {
        auto& var = POOL_MAP[type];
        while (!var.empty())
        {
            delete[] var.front();
            var.pop();
        }
    }
}


template <class T,int Flag>
void PODArrayPool<T,Flag>::clear()
{
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& pool_map : POOL_MAP)
    {
        while (!pool_map.second.empty())
        {
            delete[] pool_map.second.front();
            pool_map.second.pop();
        }
    }
}

template <class T,int Flag>
int PODArrayPool<T,Flag>::size()
{
    int size = 0;
    for (auto& pool_map : POOL_MAP)
    {
        size += pool_map.second.size() * pool_map.first;
    }
    return size;
}

template <class T,int Flag>
bool PODArrayPool<T,Flag>::contain_type(int type)
{
    return POOL_MAP.contains(type);
}


template <ResetInterface T>
T* Pool<T>::create()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (POOL_QUEUE.empty())
    {
        POOL_QUEUE.emplace(new T());
    }
    auto front = POOL_QUEUE.front();
    POOL_QUEUE.pop();
    return front;
}

template <ResetInterface T>
void Pool<T>::recycle(T* obj)
{
    std::lock_guard<std::mutex> lock(mtx);
    obj->reset();
    if (POOL_QUEUE.size() >= 1000)
    {
        delete obj;
    }
    else
    {
        POOL_QUEUE.emplace(obj);
    }
}

template <ResetInterface T>
void Pool<T>::clear()
{
    std::lock_guard<std::mutex> lock(mtx);
    while (POOL_QUEUE.size() > 0)
    {
        delete POOL_QUEUE.front();
        POOL_QUEUE.pop();
    }
}

template <ResetInterface T>
int Pool<T>::size()
{
    return POOL_QUEUE.size();
}

template <ResetInterface T,int Flag>
T* ArrayPool<T,Flag>::create(int size)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!POOL_MAP.contains(size))
    {
        POOL_MAP[size] = std::queue<T*>();
    }
    auto& queue = POOL_MAP[size];
    if (queue.empty())
    {
        queue.emplace(new T[size]);
    }
    auto front = queue.front();
    queue.pop();
    return front;
}

template <ResetInterface T,int Flag>
void ArrayPool<T,Flag>::recycle(T* obj, int size)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!POOL_MAP.contains(size))
    {
        return;
    }
    auto& queue = POOL_MAP[size];
    for (int i = 0; i < size; ++i)
    {
        obj[i].reset();
    }
    if (queue.size() >= 10)
    {
        delete[] obj;
    }
    else
    {
        queue.push(obj);
    }
}


template <ResetInterface T,int Flag>
void ArrayPool<T,Flag>::clear()
{
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& pool_map : POOL_MAP)
    {
        while (!pool_map.second.empty())
        {
            delete[] pool_map.second.front();
            pool_map.second.pop();
        }
    }
}
template <ResetInterface T,int Flag>
void ArrayPool<T,Flag>::clear(int type)
{
    if (POOL_MAP.contains(type))
    {
        auto& var = POOL_MAP[type];
        while (!var.empty())
        {
            delete[] var.front();
            var.pop();
        }
    }
}

template <ResetInterface T,int Flag>
int ArrayPool<T,Flag>::size()
{
    int size = 0;
    for (auto& pool_map : POOL_MAP)
    {
        size += pool_map.second.size() * pool_map.first;
    }
    return size;
}

template <ResetInterface T,int Flag>
bool ArrayPool<T,Flag>::contain_type(int type)
{
    return POOL_MAP.contains(type);
}


template<class T>
inline PODPool<T> PODPool<T>::POOL;

template<class T,int Flag>
inline PODArrayPool<T,Flag> PODArrayPool<T,Flag>::POOL;

template<ResetInterface T>
inline Pool<T> Pool<T>::POOL;

template<ResetInterface T,int Flag>
inline ArrayPool<T,Flag> ArrayPool<T,Flag>::POOL;
#endif //POOL_H



