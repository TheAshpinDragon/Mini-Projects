// Inspiration: https://www.youtube.com/watch?v=RZBhSi_PwHU

#include <numeric>
#include <chrono>
#include <string>

#define SAMPLE_SIZE  10000000
#define RAND_NUM_MAX 100000000

// Tests if two nums are co-prime
bool coprime(int a, int b)
{
	if (std::gcd(a, b) == 1)
		return true;

	return false;
}

// Random number bounded by RAND_NUM_MAX
int getRandInt() 
{
	return (int)(rand() % RAND_NUM_MAX);
}

// Neat little helper to format
std::string formatIntWithSeparator(int num, char separator = ',')
{
	std::string out;
	std::string numStr = std::to_string(num);
	int numDigits = numStr.size();

	for (int i = 0; i < numDigits; i++)
	{
		if ((numDigits - i) % 3 == 0 && i != 0) out += separator;
		out += numStr[i];
	}

	return out;
}

int main()
{
	// init clock and seed rand
	std::chrono::high_resolution_clock c;
	srand((unsigned int)(c.now().time_since_epoch().count()));

	// Init co-prime count
	int coPrime = 0;

	// Start time
	std::chrono::time_point t1 = c.now();

	// Loop SAMPLE_SIZE times and tally co-primes
	for (int i = 0; i < SAMPLE_SIZE; i++)
	{
		if (coprime(getRandInt(), getRandInt())) coPrime++;
	}

	// End time
	std::chrono::time_point t2 = c.now();

	// Durration of calculations
	float durration = float(std::chrono::duration_cast <std::chrono::microseconds> (t2 - t1).count()) / 1000;
	
	// x = 6 / pi^2
	// pi^2 = 6 / x
	// pi = ./(6 / x)

	// x = co-primes / total samples
	float x = (1.0f * coPrime) / (1.0f * SAMPLE_SIZE);
	// calc pi from equation
	float pi = sqrt(6.0 / x);
	// Percent error
	float percentError = ((pi - 3.141592653589) / 3.141592653589) * 100;

	// Results
	std::printf("Finished in %f(ms).\nComputed %s values.\nFound %s co-prime pairs.\nx = coprime / sample size = %f.\npi = ./(6 / x) = %f.\nPercent error = %f%%.\n\n\n",
				 durration, formatIntWithSeparator(SAMPLE_SIZE * 2).c_str(), formatIntWithSeparator(coPrime).c_str(), x, pi, percentError);
}