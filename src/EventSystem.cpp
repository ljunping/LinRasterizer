//
// Created by Lin on 2024/12/17.
//

#include "EventSystem.h"

std::unordered_map<EventType, std::unordered_set<EventFunc>> EventSystem::events;
std::unordered_map<void (*)(EventParam& param, void* userdata), std::unordered_set<void*>> EventSystem::event_param;



void EventSystem::register_listener(EventType type, void(* listener)(EventParam& param, void* userdata), void* data)
{
    events[type].insert(listener);
    event_param[listener].insert(data);
}

void EventSystem::unregister_listener(EventType type, EventFunc listener,void* data)
{
    events[type].erase(listener);
    event_param[listener].erase(data);
}

void EventSystem::dispatch_event(EventType type, EventParam& param)
{
    for (auto event : events[type])
    {
        for (auto data : event_param[event])
        {
            event(param, data);
        }
    }
}
