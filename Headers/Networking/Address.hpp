#pragma once

#include <string>

#include "Core/Types.hpp"
#include "Sockets.hpp"

namespace Networking
{
	class Address
	{
	public:
		enum class Type : u8 {
			None,
			IPv4,
			IPv6,
		};
	public:
		Address() = default;
		Address(const Address&);
		Address& operator=(const Address&);
		~Address() = default;

		Address(const std::string& ip, uint16_t port);
		Address(const sockaddr_storage& addr);

		static Address Any(Type type, u16 port);
		static Address Loopback(Type type, u16 port);

		bool operator==(const Address& other) const;
		bool operator!=(const Address& other) const { return !(*this == other); }

		Type type() const { return mType; }
		bool isValid() const { return mType != Type::None; }
		std::string address() const;
		uint16_t port() const { return mPort; }
		std::string toString() const;

		// Connecte le socket en paramètre à l’adresse interne
			// Retourne true si la connexion réussit ou débute (socket non bloquant), false sinon
		bool connect(SOCKET sckt) const;
		// Accepte une connexion entrante sur le socket en paramètre, puis met à jour l’adresse interne avec celle de l’émetteur
		// Retourne true si un nouveau socket a été accepté et met à jour newClient avec le nouveau socket, false sinon
		bool accept(SOCKET sckt, SOCKET& newClient);
		// Assigne l’adresse interne au socket en paramètre
		bool bind(SOCKET sckt) const;
		// Envoie des données depuis le socket en paramètre vers l’adresse interne
		int sendTo(SOCKET sckt, const char* data, size_t datalen) const;
		// Reçoit des données depuis le socket en paramètre puis met à jour l’adresse interne avec celle de l’émetteur
		int recvFrom(SOCKET sckt, u8* buffer, size_t bufferSize);

	private:
		void set(const sockaddr_storage& src);

		sockaddr_storage mStorage = { 0 };
		uint16_t mPort = 0;
		Type mType = Type::None;
	};
}