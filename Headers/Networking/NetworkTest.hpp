#pragma once

#include "UDP/Protocols/ReliableOrdered.hpp"
#include "Core/Debug/Log.hpp"

#include <limits>
#include <chrono>
#include <random>

#define CHECK(value) if (!(value)) {DEBUG_LOG("ERROR\n")}

class ReliableOrdered_Multiplexer_Test
{
public:
	static void Test();
};

void ReliableOrdered_Multiplexer_Test::Test()
{
	Networking::UDP::Protocols::RMultiplexer mux;
	CHECK(mux.mQueue.size() == 0);
	CHECK(mux.mNextId == 0);
	CHECK(mux.mFirstAllowedPacket == 0);
	Networking::UDP::Datagram::ID datagramId = 0;
	{
		const std::array<uint8_t, 5> arr{ 'T', 'o', 't', 'o', '\0' };
		std::vector<uint8_t> data(arr.cbegin(), arr.cend());
		mux.queue(std::move(data));
		CHECK(mux.mQueue.size() == 1);
		CHECK(mux.mNextId == 1);
		CHECK(mux.mFirstAllowedPacket == 0);

		const Networking::UDP::Datagram::ID sentDatagramId = datagramId;
		std::array<uint8_t, Networking::UDP::Protocols::Packet::PacketMaxSize> buffer;
		const size_t serializedData = mux.serialize(buffer.data(), buffer.size(), datagramId++);
		CHECK(serializedData == Networking::UDP::Protocols::Packet::HeaderSize + arr.size());
		const Networking::UDP::Protocols::Packet* packet = reinterpret_cast<const Networking::UDP::Protocols::Packet*>(buffer.data());
		CHECK(packet->datasize() == arr.size());
		CHECK(memcmp(packet->data(), arr.data(), packet->datasize()) == 0);
		//!< Le message reste en file d’envoi jusqu’à acquittement
		CHECK(mux.mQueue.size() == 1);
		//!< Acquittement de ce datagramme
		mux.onDatagramAcked(sentDatagramId);
		CHECK(mux.mQueue.size() == 0);
		CHECK(mux.mFirstAllowedPacket == 1);
	}
	{
		//!< Envoi d’un message en 3 fragments
		std::vector<uint8_t> data(Networking::UDP::Protocols::Packet::DataMaxSize * 3, 0);
		const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
		std::default_random_engine generator(seed);
		std::uniform_int_distribution<unsigned int> distribution(0, 100);
		for (uint8_t& d : data)
			d = distribution(generator);

		const auto datacopy = data;
		mux.queue(std::move(data));
		CHECK(mux.mQueue.size() == 3);
		CHECK(mux.mNextId == 4);
		size_t totalDataSize = 0;
		const Networking::UDP::Datagram::ID firstSentDatagramId = datagramId;
		const size_t nbPackets = mux.mQueue.size();
		for (;;)
		{
			std::array<uint8_t, Networking::UDP::Protocols::Packet::PacketMaxSize> buffer;
			const size_t serializedData = mux.serialize(buffer.data(), buffer.size(), datagramId++);
			if (serializedData == 0)
				break;
			const Networking::UDP::Protocols::Packet* packet = reinterpret_cast<const Networking::UDP::Protocols::Packet*>(buffer.data());
			CHECK(memcmp(packet->data(), datacopy.data() + totalDataSize, packet->datasize()) == 0);
			totalDataSize += serializedData - Networking::UDP::Protocols::Packet::HeaderSize;
		}
		CHECK(totalDataSize == datacopy.size());
		CHECK(mux.mQueue.size() == 3);
		//!< Acquittement de chacun des datagrammes
		size_t acked = 0;
		for (auto i = 0; i < nbPackets; ++i)
		{
			const Networking::UDP::Datagram::ID dgramId = firstSentDatagramId + i;
			mux.onDatagramAcked(dgramId);
			++acked;
			CHECK(mux.mQueue.size() == 3 - acked);
		}
		CHECK(mux.mFirstAllowedPacket == 4);
	}
	{
		//!< Envoi d’un message qui va être perdu
		const std::array<uint8_t, 5> arr{ 'T', 'i', 't', 'i', '\0' };
		std::vector<uint8_t> data(arr.cbegin(), arr.cend());
		mux.queue(std::move(data));
		CHECK(mux.mQueue.size() == 1);
		const Networking::UDP::Datagram::ID lostDatagramId = datagramId;

		{
			std::array<uint8_t, Networking::UDP::Protocols::Packet::PacketMaxSize> buffer;
			const size_t serializedData = mux.serialize(buffer.data(), buffer.size(), datagramId++);
			CHECK(serializedData == Networking::UDP::Protocols::Packet::HeaderSize + arr.size());
			const Networking::UDP::Protocols::Packet* packet = reinterpret_cast<const Networking::UDP::Protocols::Packet*>(buffer.data());
			CHECK(packet->datasize() == arr.size());
			CHECK(memcmp(packet->data(), arr.data(), packet->datasize()) == 0);
		}
		CHECK(mux.mQueue.size() == 1);
		CHECK(mux.mFirstAllowedPacket == 4);

		//!< Le paquet n’a pas encore été perdu, on ne devrait rien avoir à sérialiser
		{
			std::array<uint8_t, Networking::UDP::Protocols::Packet::PacketMaxSize> buffer;
			const size_t serializedData = mux.serialize(buffer.data(), buffer.size(), datagramId++);
			CHECK(serializedData == 0);
		}
		CHECK(mux.mQueue.size() == 1);
		mux.onDatagramLost(lostDatagramId);
		CHECK(mux.mFirstAllowedPacket == 4);

		const Networking::UDP::Datagram::ID datagramIdToAck = datagramId;
		{
			//!< Renvoi
			std::array<uint8_t, Networking::UDP::Protocols::Packet::PacketMaxSize> buffer;
			const size_t serializedData = mux.serialize(buffer.data(), buffer.size(), datagramId++);
			CHECK(serializedData == Networking::UDP::Protocols::Packet::HeaderSize + arr.size());
			const Networking::UDP::Protocols::Packet* packet = reinterpret_cast<const Networking::UDP::Protocols::Packet*>(buffer.data());
			CHECK(packet->datasize() == arr.size());
			CHECK(memcmp(packet->data(), arr.data(), packet->datasize()) == 0);
		}
		CHECK(mux.mQueue.size() == 1);
		mux.onDatagramAcked(datagramIdToAck);
		//!< Il est maintenant acquitté
		CHECK(mux.mQueue.size() == 0);
		CHECK(mux.mFirstAllowedPacket == 5);
	}
	{
		//!< 3 messages, perte du second
		std::vector<uint8_t> data(Networking::UDP::Protocols::Packet::DataMaxSize * 3, 0);
		const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
		std::default_random_engine generator(seed);
		std::uniform_int_distribution<unsigned int> distribution(0, 100);
		for (uint8_t& d : data)
			d = distribution(generator);

		const auto datacopy = data;
		mux.queue(std::move(data));
		CHECK(mux.mQueue.size() == 3);
		CHECK(mux.mFirstAllowedPacket == 5);

		const auto datagram1 = datagramId;
		std::array<uint8_t, Networking::UDP::Protocols::Packet::PacketMaxSize> buffer;
		mux.serialize(buffer.data(), buffer.size(), datagramId++);
		const auto datagram2 = datagramId;
		mux.serialize(buffer.data(), buffer.size(), datagramId++);
		const auto datagram3 = datagramId;
		mux.serialize(buffer.data(), buffer.size(), datagramId++);
		CHECK(mux.mFirstAllowedPacket == 5);
		mux.onDatagramAcked(datagram3);
		CHECK(mux.mFirstAllowedPacket == 5);
		mux.onDatagramAcked(datagram2);
		CHECK(mux.mFirstAllowedPacket == 5);
		mux.onDatagramAcked(datagram1);
		CHECK(mux.mFirstAllowedPacket == 8);
	}
}

class ReliableOrdered_Demultiplexer_Test
{
public:
	static void Test();
};

void ReliableOrdered_Demultiplexer_Test::Test()
{
	//!< Utiliser le multiplexeur pour facilement gérer la file d’envoi
	//!< Il a été testé auparavant, donc il est fiable d’utilisation
	Networking::UDP::Protocols::RMultiplexer mux;
	Networking::UDP::Protocols::RDemultiplexer demux;
	CHECK(demux.mLastProcessed == std::numeric_limits<Networking::UDP::Protocols::Packet::ID>::max());
	Networking::UDP::Datagram::ID datagramId = 0;
	{
		const std::array<uint8_t, 5> arr0{ 'T', 'o', 't', 'o', '\0' };
		std::vector<uint8_t> data0(arr0.cbegin(), arr0.cend());
		//!< Réception paquets 0 & 1
		{
			mux.queue(std::move(data0));
			Networking::UDP::Protocols::Packet packet;
			mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			CHECK(packet.id() == 0);
			CHECK(packet.type() == Networking::UDP::Protocols::Packet::Type::Packet);
			CHECK(packet.datasize() == static_cast<uint16_t>(arr0.size()));
			CHECK(memcmp(packet.data(), arr0.data(), arr0.size()) == 0);
			demux.onDataReceived(packet.buffer(), packet.size());
		}
		const std::array<uint8_t, 5> arr1{ 'T', 'a', 't', 'a', '\0' };
		std::vector<uint8_t> data1(arr1.cbegin(), arr1.cend());
		{
			mux.queue(std::move(data1));
			Networking::UDP::Protocols::Packet packet;
			mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			CHECK(packet.id() == 1);
			CHECK(packet.type() == Networking::UDP::Protocols::Packet::Type::Packet);
			CHECK(packet.datasize() == static_cast<uint16_t>(arr1.size()));
			CHECK(memcmp(packet.data(), arr1.data(), arr1.size()) == 0);
			demux.onDataReceived(packet.buffer(), packet.size());
		}
		const std::vector<std::vector<uint8_t>> packets = demux.process();
		CHECK(packets.size() == 2);
		CHECK(demux.mLastProcessed == 1);
		CHECK(packets[0].size() == data0.size());
		CHECK(packets[0] == data0);
		CHECK(packets[1].size() == data1.size());
		CHECK(packets[1] == data1);
	}
	{
		//!< Réception d’un message fragmenté : 3 fragments
		std::vector<uint8_t> data(Networking::UDP::Protocols::Packet::DataMaxSize * 3, 0);
		const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
		std::default_random_engine generator(seed);
		std::uniform_int_distribution<unsigned int> distribution(0, 100);
		for (uint8_t& d : data)
			d = distribution(generator);

		const auto datacopy = data;
		mux.queue(std::move(data));
		{
			const auto sentDatagramdId = datagramId;
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			demux.onDataReceived(packet.buffer(), packet.size());
			CHECK(demux.process().empty());
			mux.onDatagramAcked(sentDatagramdId);
		}
		{
			const auto sentDatagramdId = datagramId;
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			demux.onDataReceived(packet.buffer(), packet.size());
			CHECK(demux.process().empty());
			mux.onDatagramAcked(sentDatagramdId);
		}
		{
			const auto sentDatagramdId = datagramId;
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			demux.onDataReceived(packet.buffer(), packet.size());
			mux.onDatagramAcked(sentDatagramdId);
		}
		{
			const auto sentDatagramdId = datagramId;
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			CHECK(serializedData == 0);
			mux.onDatagramAcked(sentDatagramdId);
		}
		const std::vector<std::vector<uint8_t>> packets = demux.process();
		CHECK(packets.size() == 1);
		CHECK(demux.mLastProcessed == 4);
		CHECK(packets[0].size() == datacopy.size());
		CHECK(packets[0] == datacopy);
	}
	{
		//!< Réception d’un message fragmenté : 3 fragments, perte du second fragment
		std::vector<uint8_t> data(Networking::UDP::Protocols::Packet::DataMaxSize * 3, 0);
		const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
		std::default_random_engine generator(seed);
		std::uniform_int_distribution<unsigned int> distribution(0, 100);
		for (uint8_t& d : data)
			d = distribution(generator);

		const auto datacopy = data;
		mux.queue(std::move(data));
		{
			const auto sentDatagramdId = datagramId;
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			demux.onDataReceived(packet.buffer(), packet.size());
			CHECK(demux.process().empty());
			mux.onDatagramAcked(sentDatagramdId);
		}
		const auto lostDatagramdId = datagramId;
		{
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
		}
		{
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			demux.onDataReceived(packet.buffer(), packet.size());
		}
		{
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			CHECK(serializedData == 0);
		}
		{
			mux.onDatagramLost(lostDatagramdId);
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize, datagramId++);
			demux.onDataReceived(packet.buffer(), packet.size());
		}
		const std::vector<std::vector<uint8_t>> packets = demux.process();
		CHECK(packets.size() == 1);
		CHECK(demux.mLastProcessed == 7);
		CHECK(packets[0].size() == datacopy.size());
		CHECK(packets[0] == datacopy);
	}
}

class ReliableOrdered_Test
{
public:
	static void Test()
	{
		ReliableOrdered_Multiplexer_Test::Test();
		ReliableOrdered_Demultiplexer_Test::Test();
	}
};