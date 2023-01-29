#pragma once

#include <vector>

#include "NetworkSettings.hpp"
#include "Core/Types.hpp"
#include "Address.hpp"

namespace Networking::Messages
{
#define DECLARE_MESSAGE(name) friend class Base; static const Base::Type StaticType = Base::Type::name
	class Base
	{
	public:
		template<class M>
		bool is() const { return mType == M::StaticType; }
		template<class M>
		M* as() { return static_cast<M*>(this); }
		template<class M>
		const M* as() const { return static_cast<const M*>(this); }

		const Address& emitter() const { return mFrom; }
		u64 emmiterId() const { return mFromID; }

		Base(const Base&) = delete;
		Base& operator=(const Base&) = delete;

		enum class Type {
			IncomingConnection,
			Connection,
			ConnectionInterrupted,
			ConnectionResumed,
			Disconnection,
			UserData,
		};
		Type getType() const { return mType; }
	protected:
		Base(Type type, const Address& emitter, const u64& emitterID)
			: mType(type), mFrom(emitter), mFromID(emitterID)
		{}
	private:
		Type mType;
		Address mFrom;
		u64 mFromID = 0;
	};

	class IncomingConnection : public Base
	{
		DECLARE_MESSAGE(IncomingConnection);
	public:
		IncomingConnection(const IncomingConnection&) = delete;
		IncomingConnection& operator=(const IncomingConnection&) = delete;
		IncomingConnection(const Address& emitter, uint64_t emitterid) : Base(Type::IncomingConnection, emitter, emitterid) {}
	};

	class Connection : public Base
	{
		DECLARE_MESSAGE(Connection);
	public:
		enum class Result {
			Success,
			Failed,
			Refused,
			TimedOut,
		};
		Connection(const Connection&) = delete;
		Connection& operator=(const Connection&) = delete;
		Connection(const Address& emitter, u64 emitterid, Result r) : Base(Type::Connection, emitter, emitterid), result(r) {}

		Result result;
	};

#if NETWORK_INTERRUPTION
	class ConnectionInterrupted : public Base
	{
		DECLARE_MESSAGE(ConnectionInterrupted);
	public:
		ConnectionInterrupted(const ConnectionInterrupted&) = delete;
		ConnectionInterrupted& operator=(const ConnectionInterrupted&) = delete;
		ConnectionInterrupted(const Address& emitter, u64 emitterid, bool isDirect) : Base(Type::ConnectionInterrupted, emitter, emitterid), isDirectInterruption(isDirect) {}
		// True if the emitter is directly interrupted to us. False if the emitter forwarded an interruption on his side.
		bool isDirectInterruption;
	};

	class ConnectionResumed : public Base
	{
		DECLARE_MESSAGE(ConnectionResumed);
	public:
		ConnectionResumed(const ConnectionResumed&) = delete;
		ConnectionResumed& operator=(const ConnectionResumed&) = delete;
		ConnectionResumed(const Address& emitter, u64 emitterid, bool networkResume) : Base(Type::ConnectionResumed, emitter, emitterid), isNetworkResumed(networkResume) {}
		// True if the network is now completely resumed. False if network is not yet resumed due to another client being interrupted.
		bool isNetworkResumed;
	};
#endif

	class Disconnection : public Base
	{
		DECLARE_MESSAGE(Disconnection);
	public:
		enum class Reason {
			Disconnected,
			Lost,
		};
		Disconnection(const Disconnection&) = delete;
		Disconnection& operator=(const Disconnection&) = delete;
		Disconnection(const Address& emitter, u64 emitterid, Reason r) : Base(Type::Disconnection, emitter, emitterid), reason(r) {}

		Reason reason;
	};

	class UserData : public Base
	{
		DECLARE_MESSAGE(UserData);
	public:
		UserData(const UserData&) = delete;
		UserData& operator=(const UserData&) = delete;
		UserData(const Address& emitter, u64 emitterid, std::vector<unsigned char>&& d, u8 channel) : Base(Type::UserData, emitter, emitterid), data(std::move(d)), channelId(channel) {}
		std::vector<unsigned char> data;
		u8 channelId;
	};
#undef DECLARE_MESSAGE
}
