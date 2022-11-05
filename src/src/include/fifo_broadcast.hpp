#pragma once

#include <unordered_set>

#include "uniform_reliable_broadcast.hpp"

class ReliableFIFOBroadcast final : public BestEffortBroadcast
{
private:
    struct PeerState
    {
        Broadcast::Message::Id::Seq next{1};
        std::set<Broadcast::Message::Id::Seq> pending;
    };

private:
    std::unordered_map<PerfectLink::Id, PeerState> peer_state_;

public:
    explicit ReliableFIFOBroadcast(Logger &logger, unsigned long long id) noexcept
        : BestEffortBroadcast(logger, id, true) {}

    ~ReliableFIFOBroadcast() noexcept override = default;

protected:
    void SendInternal(const Broadcast::Message &msg) noexcept final
    {
        BestEffortBroadcast::SendInternal(msg);
    }

    void NotifyInternal(const Broadcast::Message &msg) noexcept final
    {
        BestEffortBroadcast::NotifyInternal(msg);
    }

    /**
     * @brief FIFO adds one more level of asychrony
     * Instead of logging the message right away we wait
     * for the missing messages so that we guarantee the FIFO property.
     *
     * @param id
     * @param log
     */
    void DeliverInternal(const Broadcast::Message::Id &id, bool log = false) noexcept final
    {
        auto &state = peer_state_[id.author];

        std::vector<Message::Id::Seq> to_remove;
        to_remove.reserve(state.pending.size());

        state.pending.insert(id.seq);
        for (const auto seq : state.pending)
        {
            if (seq > state.next)
            {
                std::cout << "if 1\n";
                break;
            }
            else
            {
                std::cout << "if 2\n";
                ++state.next;
                to_remove.emplace_back(seq);
                if (log)
                {
                    LogDeliver(id);
                }
            }
        }

        for (const auto seq : to_remove)
            state.pending.erase(seq);
    }
};

class UniformFIFOBroadcast final : public UniformReliableBroadcast
{
private:
    struct PeerState
    {
        Broadcast::Message::Id::Seq next{1};
        std::set<Broadcast::Message::Id::Seq> pending;
    };

private:
    std::unordered_map<PerfectLink::Id, PeerState> peer_state_;

public:
    explicit UniformFIFOBroadcast(Logger &logger, unsigned long long id) noexcept
        : UniformReliableBroadcast(logger, id) {}

    ~UniformFIFOBroadcast() noexcept override = default;

protected:
    void SendInternal(const Broadcast::Message &msg) noexcept final
    {
        UniformReliableBroadcast::SendInternal(msg);
    }

    void NotifyInternal(const Broadcast::Message &msg) noexcept final
    {
        UniformReliableBroadcast::NotifyInternal(msg);
    }

    /**
     * @brief FIFO adds one more level of asychrony
     * Instead of logging the message right away we wait
     * for the missing messages so that we guarantee the FIFO property.
     *
     * @param id
     * @param log
     */
    void DeliverInternal(const Broadcast::Message::Id &id, bool log = false) noexcept final
    {
        auto &state = peer_state_[id.author];

        std::vector<Message::Id::Seq> to_remove;
        to_remove.reserve(state.pending.size());

        state.pending.insert(id.seq);
        for (const auto seq : state.pending)
        {
            if (seq > state.next)
            {
                break;
            }
            else
            {
                state.next++;
                to_remove.emplace_back(seq);

                if (log)
                {
                    LogDeliver(id);
                }
            }
        }

        for (const auto seq : to_remove)
            state.pending.erase(seq);
    }
};