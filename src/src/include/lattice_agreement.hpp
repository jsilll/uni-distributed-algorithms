#pragma once

#include "logger.hpp"
#include "best_effort_broadcast.hpp"

#include <queue>

class LatticeAgreement final : public BestEffortBroadcast
{
private:
  struct Proposal
  {
    typedef unsigned int Number;

    Number number;
    std::unordered_set<unsigned int> values;
  };

  struct Message
  {
    enum Type
    {
      kProposal,
      kAck,
      kNack,
    };

    Type type;
    Proposal proposal;
    unsigned int round;
  };

  struct ProposalState
  {
    Proposal proposal;
    bool active{false};
    unsigned int ack_count{1};
    unsigned int nack_count{0};
    std::unordered_set<unsigned int> accepted{proposal.values};
  };

public:
  static constexpr int kPacketPrefixSize = sizeof(Message::Type) +
                                           sizeof(unsigned int) +
                                           sizeof(Proposal::Number);
private:
  std::atomic_uint current_round_{0};
  Shared<ProposalState> current_proposal_state_{};
  Shared<std::queue<std::unordered_set<unsigned int>>> to_propose_{};
  Shared<std::unordered_map<unsigned int, ProposalState>> agreed_proposals_{};
  Shared<std::unordered_map<unsigned int, std::queue<std::pair<PerfectLink::Id, Message>>>> ahead_of_time_messages_{};

  std::thread agreement_checker_thread_;

public:
  LatticeAgreement(Logger &logger, PerfectLink::Id id) noexcept
      : BestEffortBroadcast::BestEffortBroadcast(logger, id) {}

  ~LatticeAgreement() noexcept override = default;

  inline void Start() noexcept override
  {
    Broadcast::Start();
#ifdef DEBUG
    std::cout << "[DBUG] Creating new thread: LatticeAgreement::CheckForAgreement\n";
#endif
    agreement_checker_thread_ = std::thread(&LatticeAgreement::CheckForAgreement, this);
  }

  inline void Stop() noexcept override
  {
    if (on_.load())
    {
      Broadcast::Stop();
      agreement_checker_thread_.join();
    }
  }

  void Propose(const std::vector<unsigned int> &proposed) noexcept;

  void NotifyInternal(const Broadcast::Message &msg) noexcept override;

private:
  void CheckForAgreement() noexcept;

  void Decide(std::unordered_set<unsigned int> &values) noexcept;

  static std::optional<Message> Parse(const std::vector<char> &bytes) noexcept;

  std::optional<Broadcast::Message> HandleMessage(const Message &msg,
                                                  ProposalState &proposal_state) noexcept;

  static std::size_t Serialize(Message::Type type,
                               unsigned int round,
                               Proposal::Number number,
                               const std::unordered_set<unsigned int> &proposed,
                               std::vector<char> &buffer) noexcept;
};