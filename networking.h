#pragma once

#ifndef NETWORKING_H
#define NETWORKING_H

#include <windows.h>
#include <vector>

extern "C" {
	#include "lib/ZRTPCPP/cryptcommon/twofish.h"
}
#include "logger.h"

#pragma comment(lib, "Ws2_32.lib")

namespace Networking {

	class connection {

		#define START 1
		#define END -1
		
	public:
		
		connection(int port);
		~connection();

		bool sendMessage(const char* message);
		bool recieveMessage(std::string& message);
		void changePort(int port);

		void endConnection();



	private:
		int port;
		BOOL broadcastEnable;
		SOCKET sockfd;
		sockaddr_in addr;
	};

	class encryption {

	public:

		encryption();

		~encryption();

		void encrypt(Twofish_Byte p[16], Twofish_Byte c[16]);

		void decrypt(Twofish_Byte c[16], Twofish_Byte p[16]);

	private:

		int gcd(int a, int b);

		int expmod(int base, int exp, int mod);

		bool trial_composite(int round_tester, int even_component, int miller_rabin_canidate, int max_divsion_by_two); 

		bool is_miller_rabin_passed(int miller_rabin_canidate);

		int rand_prime_num();

		void generate_key(Twofish_Byte internal_key[32]);

		Twofish_key xkey;

	};


}


#endif