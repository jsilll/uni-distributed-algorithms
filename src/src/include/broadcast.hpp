#pragma once

#include "logger.hpp"
#include "perfect_link.hpp"

class Broadcast : public PerfectLink::Manager
{
public:
  struct Message
  {
    struct Id
    {
      typedef unsigned int Seq;

      Seq seq;
      PerfectLink::Id author;

      inline bool friend operator<(const Id &id1, const Id &id2) noexcept
      {
        if (id1.seq != id2.seq)
        {
          return id1.seq < id2.seq;
        }

        return id1.author < id2.author;
      }

      inline bool friend operator==(const Id &id1, const Id &id2) noexcept
      {
        return id1.author == id2.author && id1.seq == id2.seq;
      }
    };

    Id id;
    PerfectLink::Id sender;
    std::vector<char> payload;
  };

protected:
  static constexpr size_t kPacketPrefixSize = sizeof(PerfectLink::Id) + sizeof(Message::Id::Seq);

protected:
  PerfectLink::Id id_;
  std::atomic<Message::Id::Seq> n_messages_sent_{1};

public:
  explicit Broadcast(Logger &logger, PerfectLink::Id id) noexcept
      : PerfectLink::Manager::Manager(logger), id_(id) {}

  ~Broadcast() noexcept override = default;

  void Send(const std::string &msg) noexcept;

protected:
  void Notify(PerfectLink::Id sender_id, const PerfectLink::Message &msg) noexcept final;

protected:
  virtual void SendInternal(const Broadcast::Message &msg) = 0;
  virtual void NotifyInternal(const Broadcast::Message &msg) = 0;
  virtual void DeliverInternal(const Broadcast::Message::Id &id, bool log) = 0;

protected:
  void LogSend(Broadcast::Message::Id::Seq seq) noexcept;
  void LogDeliver(const Broadcast::Message::Id &id) noexcept;

public:
  static std::size_t Serialize(const Broadcast::Message &msg, char *buffer) noexcept;
  static std::optional<Message> Parse(PerfectLink::Id sender_id, const std::vector<char> &bytes) noexcept;
};

template <>
struct std::hash<Broadcast::Message::Id>
{
  inline std::size_t operator()(const Broadcast::Message::Id &id) const noexcept
  {
    std::size_t h1 = std::hash<PerfectLink::Id>{}(id.author);
    std::size_t h2 = std::hash<Broadcast::Message::Id::Seq>{}(id.seq);
    return h1 ^ (h2 << 1);
  }
};