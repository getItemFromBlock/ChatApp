#pragma once

#include "UDP/Protocols/UnreliableOrdered.hpp"

#include <limits>
#include <chrono>
#include <random>
#include <iostream>

#define CHECK(value) if (!(value)) {std::cout <<"ERROR" << std::endl;}

class Multiplexer_Test
{
public:
	static void Test();
};

void Multiplexer_Test::Test()
{
	Networking::UDP::Protocols::UMultiplexer mux;
	CHECK(mux.mQueue.size() == 0);
	CHECK(mux.mNextId == 0);
	{
		const std::array<uint8_t, 5> arr{ 'T', 'o', 't', 'o', '\0' };
		std::vector<uint8_t> data(arr.cbegin(), arr.cend());
		mux.queue(std::move(data));
		CHECK(mux.mQueue.size() == 1);
		CHECK(mux.mNextId == 1);

		std::array<uint8_t, Networking::UDP::Protocols::Packet::PacketMaxSize> buffer;
		const size_t serializedData = mux.serialize(buffer.data(), buffer.size());
		CHECK(serializedData == Networking::UDP::Protocols::Packet::HeaderSize + arr.size());
		const Networking::UDP::Protocols::Packet* packet = reinterpret_cast<const Networking::UDP::Protocols::Packet*>(buffer.data());
		CHECK(packet->datasize() == arr.size());
		CHECK(memcmp(packet->data(), arr.data(), packet->datasize()) == 0);
	}
	{
		//!< Envoi d’un message fragmenté : 3 fragments
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
		for (; !mux.mQueue.empty(); )
		{
			std::array<uint8_t, Networking::UDP::Protocols::Packet::PacketMaxSize> buffer;
			const size_t serializedData = mux.serialize(buffer.data(), buffer.size());
			const Networking::UDP::Protocols::Packet* packet = reinterpret_cast<const Networking::UDP::Protocols::Packet*>(buffer.data());
			CHECK(memcmp(packet->data(), datacopy.data() + totalDataSize, packet->datasize()) == 0);
			totalDataSize += serializedData - Networking::UDP::Protocols::Packet::HeaderSize;
		}
		CHECK(totalDataSize == datacopy.size());
	}
}

class Demultiplexer_Test
{
public:
	static void Test();
};

void Demultiplexer_Test::Test()
{
	//!< Utilisation du multiplexer pour facilement mettre en file et découper les données
	//!< Il est testé au préalable donc son utilisation est fiable
	Networking::UDP::Protocols::UMultiplexer mux;
	Networking::UDP::Protocols::UDemultiplexer demux;
	//CHECK(demux.mPendingQueue.empty());
	CHECK(demux.mLastProcessed == std::numeric_limits<Networking::UDP::Protocols::Packet::ID>::max());
	{
		const std::array<uint8_t, 5> arr0{ 'T', 'o', 't', 'o', '\0' };
		std::vector<uint8_t> data0(arr0.cbegin(), arr0.cend());
		//!< Réception des paquets 0 & 1
		{
			mux.queue(std::move(data0));
			Networking::UDP::Protocols::Packet packet;
			mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize);
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
			mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize);
			CHECK(packet.id() == 1);
			CHECK(packet.type() == Networking::UDP::Protocols::Packet::Type::Packet);
			CHECK(packet.datasize() == static_cast<uint16_t>(arr1.size()));
			CHECK(memcmp(packet.data(), arr1.data(), arr1.size()) == 0);
			demux.onDataReceived(packet.buffer(), packet.size());
		}
		CHECK(demux.mPendingQueue.size() == 2);
		const std::vector<std::vector<uint8_t>> packets = demux.process();
		CHECK(packets.size() == 2);
		CHECK(demux.mLastProcessed == 1);
		CHECK(packets[0].size() == data0.size());
		CHECK(packets[0] == data0);
		CHECK(packets[1].size() == data1.size());
		CHECK(packets[1] == data1);
	}
	{
		//!< Réception d’un message fragmenté : 3 fragments
		std::vector<uint8_t> data(Networking::UDP::Protocols::Packet::DataMaxSize * 3, 0);
		const unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
		std::default_random_engine generator(seed);
		std::uniform_int_distribution<unsigned int> distribution(0, 100);
		for (uint8_t& d : data)
			d = distribution(generator);

		const auto datacopy = data;
		mux.queue(std::move(data));
		for (;;)
		{
			Networking::UDP::Protocols::Packet packet;
			const size_t serializedData = mux.serialize(reinterpret_cast<uint8_t*>(&packet), Networking::UDP::Protocols::Packet::PacketMaxSize);
			if (serializedData == 0)
				break;
			demux.onDataReceived(packet.buffer(), packet.size());
		}
		CHECK(demux.mPendingQueue.size() == 3);
		const std::vector<std::vector<uint8_t>> packets = demux.process();
		CHECK(packets.size() == 1);
		CHECK(demux.mLastProcessed == 4);
		CHECK(packets[0].size() == datacopy.size());
		CHECK(packets[0] == datacopy);
	}
}

class PacketHandling_Test
{
public:
	static void Test()
	{
		Multiplexer_Test::Test();
		Demultiplexer_Test::Test();
	}
};