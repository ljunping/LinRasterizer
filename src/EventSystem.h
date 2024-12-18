//
// Created by Lin on 2024/12/17.
//

#ifndef EVENTSYSTEM_H
#define EVENTSYSTEM_H
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

enum EventType
{
    WindowSizeChange,

};
struct EventParam
{
    int m_n1{};
    int m_n2{};
    long long m_l1{};
    long long m_l2{};
    float m_f1{};
    float m_f2{};
    const char* m_str1{};
    const char* m_str2{};

};
using EventFunc= void (*)(EventParam&,void*);

class EventSystem {
    static std::unordered_map<EventType, std::unordered_set<EventFunc>> events;
    static std::unordered_map<EventFunc, std::unordered_set<void*>> event_param;
public:
    static void register_listener(EventType type, EventFunc, void* data);
    static void unregister_listener(EventType type, EventFunc listener, void* data);
    static void dispatch_event(EventType type, EventParam& param);
    template<class  ... Args>
    static void dispatch_event(EventType type, Args&& ...args);
};

template<typename... Args>
struct EventParamGenerator;

// 主模板，用于统计每种类型的出现次数
template<typename First, typename... Rest>
struct EventParamGenerator<First, Rest...>
{
    template <typename... Pre>
    static void build_event_param(EventParam& param, First&& first, Rest&&... rest);
};

template<>
struct EventParamGenerator<>
{
    template <class... Pre>
    static void build_event_param(EventParam& param);
};

template <class ... Args>
void EventSystem::dispatch_event(EventType type, Args&& ...args)
{
    EventParam param;
    EventParamGenerator<Args...>::template build_event_param<>(param, args...);
    dispatch_event(type, param);
}



template<typename T, typename U>
constexpr bool isSameIgnoringConst() {
    return std::is_same_v<std::remove_reference_t<std::remove_const_t<T>>, std::remove_reference_t<std::remove_const_t<U>>>;
}

template<typename T, typename U>
struct TypeCount {
    static constexpr int value = std::is_same<T, U>::value;
};

// 递归模板，用于累加计数
template<typename T, typename... Rest>
struct CountType {
    static constexpr int value = 0;
};

template<typename T, typename First, typename... Rest>
struct CountType<T, First, Rest...> {
    static constexpr int value = TypeCount<T, First>::value + CountType<T, Rest...>::value;
};


template <typename First, typename ... Rest>
template <typename ... Pre>
void EventParamGenerator<First, Rest...>::build_event_param(EventParam& param, First&& first, Rest&&... rest)
{
    static constexpr int count = CountType<First, Pre...>::value;
    if constexpr (isSameIgnoringConst<First, int>())
    {
        if constexpr (count == 0)
        {
            param.m_n1 = first;
        }
        if constexpr (count == 1)
        {
            param.m_n2 = first;
        }
    }
    if constexpr (isSameIgnoringConst<First, long long>())
    {
        if constexpr (count == 0)
        {
            param.m_l1 = first;
        }
        if constexpr (count == 1)
        {
            param.m_l2 = first;
        }
    }

    if constexpr (isSameIgnoringConst<First, float>())
    {
        if constexpr (count == 0)
        {
            param.m_f1 = first;
        }
        if constexpr (count == 1)
        {
            param.m_f2 = first;
        }
    }

    if constexpr (isSameIgnoringConst<First,const char*>())
    {
        if constexpr (count == 0)
        {
            param.m_str1 = first;
        }
        if constexpr (count == 1)
        {
            param.m_str2 = first;
        }
    }
    EventParamGenerator<Rest...>::template build_event_param<Pre..., First>(param, std::forward<Rest>(rest)...);
}

template <class ... Pre>
void EventParamGenerator<>::build_event_param(EventParam& param)
{
}


#endif //EVENTSYSTEM_H
