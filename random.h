#pragma once

#include <vector>


namespace Random {

	class primeGenerator {

	public:

		primeGenerator(int bit_wide);

		int generate();

		~primeGenerator();

	private:

		int bit_wide;

		long random_int();
	};


}