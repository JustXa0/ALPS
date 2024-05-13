#include "networking.h"
#include <cstring>
#include "conversions.h"

// TODO: Handle errors more clearly in the future!

Networking::connection::connection(int port) {
	WSADATA wsaData;
	Networking::connection::port = port;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		Logger::systemLogger.addLog(Logger::fatal, "WSAStartup failed.");
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == INVALID_SOCKET) {
		Logger::systemLogger.addLog(Logger::fatal, "Socket creation failure.");
	}

	broadcastEnable = TRUE;
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcastEnable, sizeof(broadcastEnable)) == SOCKET_ERROR) {
		Logger::systemLogger.addLog(Logger::fatal, "Set socket option error");
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	Logger::systemLogger.addLog(Logger::info, "Socket succesfully created.");
}

Networking::connection::~connection() {
	closesocket(sockfd);
	WSACleanup();
	Logger::systemLogger.addLog(Logger::info, "Succefully cleaned up networking connections.");
}

bool Networking::connection::sendMessage(const char* message) {
	int result = sendto(sockfd, message, strlen(message), 0, (sockaddr*)&addr, sizeof(addr));
	if (result == SOCKET_ERROR) {
		Logger::systemLogger.addLog(Logger::error, "Sendto Error.");
		return false;
	}

	Logger::systemLogger.addLog(Logger::info, "Succesfully sent message.");

	return true;

}

bool Networking::connection::recieveMessage(std::string& message) {
	char buffer[1024];
	sockaddr_in senderAddr;
	int senderAddrSize = sizeof(senderAddr);

	int bytesRecieved = recvfrom(sockfd, buffer, sizeof(buffer), 0, (sockaddr*)&senderAddr, &senderAddrSize);
	if (bytesRecieved == SOCKET_ERROR) {
		Logger::systemLogger.addLog(Logger::error, "Recvfrom error.");
		return false;
	}

	// Appends message with a 
	buffer[bytesRecieved] = '\0';
	message.assign(buffer, bytesRecieved);

	return true;
}

void Networking::connection::changePort(int port) {
	Networking::connection::port = port;
	addr.sin_port = htons(port);
}

void Networking::connection::endConnection() {
	int result = sendto(sockfd, "-1", strlen("-1"), 0, (sockaddr*)&addr, sizeof(addr));
	if (result == SOCKET_ERROR) {
		Logger::systemLogger.addLog(Logger::error, "Sendto Error.");
		return;
	}

	Logger::systemLogger.addLog(Logger::info, "Succesfully closed connection.");
}

Networking::encryption::encryption() {

	Twofish_initialise();
	Twofish_Byte key[32];

	generate_key(key);

	Twofish_prepare_key(key, 32, &xkey);

}

void Networking::encryption::encrypt(Twofish_Byte p[16], Twofish_Byte c[16]) {
	Twofish_encrypt(&xkey, p, c);
}

void Networking::encryption::decrypt(Twofish_Byte c[16], Twofish_Byte p[16]) {
	Twofish_decrypt(&xkey, c, p);
}

int Networking::encryption::gcd(int a, int b) {
	int t;
	while (1) {
		t = a % b;
		if (t == 0) {
			return b;
		}
		a = b;
		b = t;
	}
}

int Networking::encryption::expmod(int base, int exp, int mod) {
	if (exp == 0) {
		return 1;
	}

	if (exp % 2 == 0) {
		return (int)pow(expmod(base, (exp / 2), mod), 2) % mod;
	}
	else {
		return (base * expmod(base, (exp - 1), mod)) % mod;
	}
}

bool Networking::encryption::trial_composite(int round_tester, int even_component, int miller_rabin_canidate, int max_division_by_two) {
	if (expmod(round_tester, even_component, miller_rabin_canidate) == 1) {
		return false;
	}
	for (int i = 0; i < max_division_by_two; i++) {
		if (expmod(round_tester, (1 << i) * even_component, miller_rabin_canidate) == miller_rabin_canidate - 1) {
			return false;
		}
	}

	return true;
}

bool Networking::encryption::is_miller_rabin_passed(int miller_rabin_canidate) {
	
	int max_divisions_by_two = 0;
	int even_component = miller_rabin_canidate - 1;

	while (even_component % 2 == 0) {
		even_component >> 1;
		max_divisions_by_two += 1;
	}

	int number_of_rabin_trials = 20;
	for (int i = 0; i < (number_of_rabin_trials); i++) {
		int round_tester = rand() * (miller_rabin_canidate - 2) + 2;

		if (trial_composite(round_tester, even_component, miller_rabin_canidate, max_divisions_by_two)) {
			return false;
		}
	}

	return true;

}

int Networking::encryption::rand_prime_num() {
	return 1;
}

void Networking::encryption::generate_key(Twofish_Byte internal_key[32]) {
	HCRYPTPROV hCryptProv;
	BYTE key[32];
	if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		if (CryptGenRandom(hCryptProv, sizeof(key), key)) {
			internal_key = key;
		}
		CryptReleaseContext(hCryptProv, 0);
	}
}

Networking::encryption::~encryption() {

}



