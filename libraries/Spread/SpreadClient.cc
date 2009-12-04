
#include "SpreadClient.h"
#include "Assert.h"
#include "sp.h"

SpreadClient::SpreadClient(
            const std::string &spread_name,
            const std::string &name
        )
{
    char pg[MAX_GROUP_NAME];
    int ret = SP_connect(spread_name.c_str(), name.c_str(), 0, 0, &mbox, pg);
    ASSERT(ret == ACCEPT_SESSION);
    private_group = pg;
}

SpreadClient::~SpreadClient() {
    Close();
}

void SpreadClient::Close() {
    if (mbox >= 0) {
        SP_disconnect(mbox);
        mbox = -1;
    }
}

int SpreadClient::Join(const std::string &group_name) {
    return SP_join(mbox, group_name.c_str());
}

int SpreadClient::Leave(const std::string &group_name) {
    return SP_leave(mbox, group_name.c_str());
}

int SpreadClient::Send(const SpreadMessage &msg) {
    service service_type;
    switch (msg.ServiceType()) {
    case SpreadMessage::UNRELIABLE:
        service_type = UNRELIABLE_MESS;
        break;
    case SpreadMessage::RELIABLE:
        service_type = RELIABLE_MESS;
        break;
    case SpreadMessage::FIFO:
        service_type = FIFO_MESS;
        break;
    case SpreadMessage::CAUSAL:
        service_type = CAUSAL_MESS;
        break;
    case SpreadMessage::AGREED:
        service_type = AGREED_MESS;
        break;
    case SpreadMessage::SAFE:
        service_type = SAFE_MESS;
        break;
    default:
        ASSERT(false);
    }
    int num_groups = msg.NumGroups();
    char groups[num_groups][MAX_GROUP_NAME];
    for (int i = 0; i < num_groups; ++i) {
        strncpy(groups[i], msg.GroupAt(i).c_str(), MAX_GROUP_NAME);
    }

    return SP_multigroup_multicast(mbox, service_type, num_groups,
            groups, msg.Type(), msg.Data().size(), &msg.Data().at(0));
}

int SpreadClient::Recv(SpreadMessage &msg) {
    service service_type = 0;
    char sender[MAX_GROUP_NAME];
    short mess_type = 0;
    int endian_mismatch = 0;
    int max_groups = 10;
    int num_groups = 0;
    if (msg.Data().size() < 256) {
        msg.Data().resize(256, 0);
    }
    bool loop = true;
    int ret = 0;
    while (loop) {
        char groups[max_groups][MAX_GROUP_NAME];
        ret = SP_receive(mbox, &service_type, sender, max_groups,
                &num_groups, groups, &mess_type, &endian_mismatch,
                msg.Data().size(), &msg.Data().at(0));
        if (ret < 0) {
            switch (ret) {
                case GROUPS_TOO_SHORT:
                    max_groups = -num_groups;
                    num_groups = 0;
                    break;
                case BUFFER_TOO_SHORT:
                    msg.Data().resize(-endian_mismatch, 0);
                    endian_mismatch = 0;
                    break;
                case CONNECTION_CLOSED:
                case ILLEGAL_MESSAGE:
                case ILLEGAL_SESSION:
                default:
                    return ret;
            }
        } else {
            loop = false;
            msg.EndianMismatch(endian_mismatch != 0);
            for (int i = 0; i < num_groups; ++i) {
                msg.AddGroup(groups[i]);
            }
            msg.Sender(sender);
            msg.Type(mess_type);
            msg.Data().resize(ret);
        }
    }
    return ret;
}

