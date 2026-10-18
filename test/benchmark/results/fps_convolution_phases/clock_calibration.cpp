#include "profile.hpp"
int main() {
	constexpr unsigned repetitions = 1000000;
	profile::start();
	for (unsigned i = 0; i < repetitions; ++i) profile::step(profile::IoSetupHeader);
	profile::emit();
}
