#pragma once

#ifndef KEYGEN_H
#define KEYGEN_H

#include <vector>

namespace Security {

	class keygen {

	public:

		keygen();

	private:

		long int generate_prime();

		std::vector<long int> sieve_of_atkin();



	};




}


#endif