//
// Created by qiming on 24-12-10.
//

#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <random>
namespace qm {
    class RandomGenerator
    {
    private:
        std::mt19937 pseudoRandomGenerator;

    public:
        RandomGenerator()
        {
            std::random_device rd;
            pseudoRandomGenerator.seed(rd());
        }
        RandomGenerator(int seed) : pseudoRandomGenerator(seed) {}
        void set_seed(unsigned int seed) { pseudoRandomGenerator.seed(seed); }

        int fast_rand(int lb, int ub) { return (pseudoRandomGenerator() % (ub - lb)) + lb; }
        int fast_rand(int ub) { return pseudoRandomGenerator() % ub; }
        int rand(int lb, int ub) { return std::uniform_int_distribution<int>(lb, ub - 1)(pseudoRandomGenerator); }
        int rand(int ub) { return std::uniform_int_distribution<int>(0, ub - 1)(pseudoRandomGenerator); }

        [[maybe_unused]] static RandomGenerator &getInstance()
        {
            static RandomGenerator instance;
            return instance;
        }
    };
}// namespace qm

#endif// RANDOM_GENERATOR_H
