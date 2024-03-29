#include "best_effort_broadcast.hpp"

void BestEffortBroadcast::SendInternal(const Broadcast::Message &msg) noexcept
{
    std::vector<PerfectLink *> pls;

    perfect_links_.mutex.lock_shared();
    pls.reserve(perfect_links_.data.size());
    for (const auto &[_, pl] : perfect_links_.data)
    {
        pls.emplace_back(pl.get());
    }
    perfect_links_.mutex.unlock_shared();

    static_assert(UDPServer::kMaxSendSize > PerfectLink::kPacketPrefixSize);
    static_assert((UDPServer::kMaxSendSize - PerfectLink::kPacketPrefixSize) > kPacketPrefixSize);

    char buffer[UDPServer::kMaxSendSize - PerfectLink::kPacketPrefixSize];
    std::size_t len = Serialize(msg, buffer);

#ifdef DEBUG
    if (msg.id.author == id_)
    {
        std::cout << "[DBUG] Best Effort Broadcast sending message " << msg.id.seq << " of size: " << len << "\n";
    }
#endif

    for (const auto pl : pls)
    {
        pl->Send(buffer, len);
    }
}

void BestEffortBroadcast::NotifyInternal(const Broadcast::Message &msg) noexcept
{
    if (deliver_to_upper_layer_)
    {
        DeliverInternal(msg.id, true);
    }
    else
    {
        BestEffortBroadcast::DeliverInternal(msg.id);
    }
}
