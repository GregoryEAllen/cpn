#pragma once
#include <string>
#include <vector>

// Every message has:
// Service Type
// One or more groups it was/will be sent to
// Message Type (short)
//
// Outgoing messages have a self discarding flag
//
// Incoming message have an indian mismatch flag
// also has a sender
//
// Messages can be ether scatter/gather or regular
// and can ether statically copy a pointer to the data
// or can hold a copy of the data

/**
 * This is just a container for a the parameters of a spread message.
 */
class SpreadMessage {
public:

    enum ServiceType_t {
        UNRELIABLE,
        RELIABLE,
        FIFO,
        CAUSAL,
        AGREED,
        SAFE
    };

    SpreadMessage()
        : service_type(UNRELIABLE),
        msg_type(0),
        self_discard(false),
        endian_mismatch(false)
    {
    }

    SpreadMessage(ServiceType_t service)
        : service_type(service),
        msg_type(0),
        self_discard(false),
        endian_mismatch(false)
    {
    }

    SpreadMessage(const std::string &group, ServiceType_t service)
        : service_type(service),
        msg_type(0),
        self_discard(false),
        endian_mismatch(false)
    {
        groups.push_back(group);
    }

    SpreadMessage &AddGroup(const std::string &group) {
        groups.push_back(group);
        return *this;
    }

    SpreadMessage &RemoveGroup(const std::string &group) {
        groups.erase(std::remove(groups.begin(), groups.end(), group), groups.end());
        return *this;
    }

    SpreadMessage &RemoveGroupAt(unsigned i) {
        groups.erase(groups.begin() + i);
        return *this;
    }
    unsigned NumGroups() const { return groups.size(); }
    const std::string &GroupAt(unsigned i) const { return groups.at(i); }

    SpreadMessage &ServiceType(ServiceType_t t) {
        service_type = t;
        return *this;
    }
    ServiceType_t ServiceType() const { return service_type; }

    SpreadMessage &SelfDiscard(bool discard) {
        self_discard = discard;
        return *this;
    }

    bool SelfDiscard() const { return self_discard; }

    SpreadMessage &EndianMismatch(bool em) {
        endian_mismatch = em;
        return *this;
    }

    bool EndianMismatch() const { return endian_mismatch; }

    SpreadMessage &Type(short type) {
        msg_type = type;
        return *this;
    }

    short Type() const { return msg_type; }

    SpreadMessage &Data(const char *m, unsigned len) {
        msg.assign(m, m + len);
        return *this;
    }

    SpreadMessage &Data(const std::vector<char> &m) {
        msg = m;
        return *this;
    }

    std::vector<char> &Data() { return msg; }
    const std::vector<char> &Data() const { return msg; }

    SpreadMessage &Sender(const std::string &s) {
        sender = s;
        return *this;
    }

    const std::string &Sender() const { return sender; }
private:
    std::vector<std::string> groups;
    std::string sender;
    ServiceType_t service_type;
    short msg_type;
    bool self_discard;
    bool endian_mismatch;
    std::vector<char> msg;
};


