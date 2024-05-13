#include "random.h"
#include <ctime>
#include <cmath>
#include <cstdlib>

// Beginning of internal function defintion


// calculates (base ^ exp) % mod
static int expmod(int base, int exp, int mod) {
	if (exp == 0) {
		return 1;
	}

	int result = 1 % mod;
	while (exp > 0) {
		if (exp % 2 == 1) {
			result = (result * base) % mod;
		}
		base = (base * base) % mod;
		exp /= 2;
	}
	return result;
}


static bool trialComposite(int round_tester, int even, int candidate, int max_div) {
	if (expmod(round_tester, even, candidate) == 1) {
		return false;
	}

	for (int i = 0; i < max_div; i++) {
		if (expmod(round_tester, (1 << i) * even, candidate) == candidate - 1) {
			return false;
		}
		return true;
	}
}


Random::primeGenerator::primeGenerator(int bit_wide) {
	Random::primeGenerator::bit_wide = bit_wide;
}

Random::primeGenerator::~primeGenerator() {
	Random::primeGenerator::bit_wide = 0;
}

int Random::primeGenerator::generate() {

	while (true) {
		int prime_num = random_int();

		if (test_prime2(test_prime1(prime_num))) {
			return prime_num;
		}
	}
}

// Begining of private function definitions

long Random::primeGenerator::random_int() {
	// Code is found from GfG website, linked below, I do plan on rewritting this in the future:
	// https://www.geeksforgeeks.org/how-to-generate-large-prime-numbers-for-rsa-algorithm/

	srand(time(NULL));

	long max = (long)powl(2, bit_wide) - 1;
	long min = (long)powl(2, bit_wide - 1) + 1;
	return min + (rand() % (max - min + 1));
}

std::vector<int> Random::primeGenerator::print_primes(int n) {
	// Code is found from GfG website, linked below, I do plan on rewritting this in the future:
	// https://www.geeksforgeeks.org/sieve-of-eratosthenes/
	
	std::vector<bool> prime(n + 1, true);

	for (int i = 2; i * i <= n; i++) {
		if (prime[i]) {
			for (int j = i * i; j <= n; j += i) {
				prime[j] = false;
			}
		}
	}

	std::vector<int> return_vector;
	
	// assign all prime numbers to vector
	for (int i = 2; i <= n; i++) {
		if (prime[i]) {
			return_vector.push_back(i);
		}
	}

	return return_vector;
}

int Random::primeGenerator::test_prime1(int prime_candidate) {
	while (true) {
		for (int divisor : print_primes(400)) {
			if (prime_candidate % divisor == 0 && (divisor * divisor) <= prime_candidate) {
				break;
			}
			else {
				return prime_candidate;
			}
		}
	}
}

bool Random::primeGenerator::test_prime2(int candidate) {
	int max_div = 0;		// May need to be adjusted in the future
	int even_component = candidate - 1;

	while (even_component % 2 == 0) {
		even_component >>= 1;
		max_div += 1;
	}

	int num_trials = 20;
	for (int i = 0; i < num_trials; i++) {
		int round_tester = rand() * (candidate - 2) + 2;

		if (trialComposite(round_tester, even_component, candidate, max_div)) {
			return false;
		}
		return true;
	}
}