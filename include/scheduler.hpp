#pragma once
#include <functional>
#include <queue> // For std:priority_queue
#include <vector> // For std::vector
using u64 = std::uint64_t;

enum class EventTypes {
    HBlank,
    EndOfLine,
    PollIRQs,
    FireNMI,
    Panic
};

struct Event {
    EventTypes type; // What kind of event is this?
    u64 timestamp = 0; // What cycle will this event be fired?

    Event (EventTypes type, u64 timestamp) : type(type), timestamp(timestamp) {}
    
    const char* name() const {
        switch (type) {
            case EventTypes::HBlank: return "H-Blank";
            case EventTypes::EndOfLine: return "End of line";
            case EventTypes::PollIRQs: return "Poll IRQs";
            case EventTypes::FireNMI: return "Fire NMI";
            case EventTypes::Panic: return "Panic";
        }
    }
};

// A function to compare 2 events based on their timestamp
static auto cmp = [](Event left, Event right) {
    return left.timestamp > right.timestamp;
};

class Scheduler {
    const int MAX_EVENT_NUM = 16; // How many events can the scheduler hold at most?
    std::priority_queue<Event, std::vector <Event>, decltype(cmp)> events; // Our queue of events

public:
    u64 timestamp = 0; // What cycle are we on?
    
    Event next() { return events.top(); } // Peek next event
    void removeNext() { events.pop(); }
    void addCycles (u64 cycles) { timestamp += cycles; }

    void pushEvent (EventTypes type, u64 cycle) {
        events.push (Event(type, cycle));
    }
    
    Scheduler() : events(cmp) {
        pushEvent (EventTypes::HBlank, 1092); // Add first event
        pushEvent (EventTypes::Panic, UINT64_MAX); // A dummy event that's always in the queue
    }
};
