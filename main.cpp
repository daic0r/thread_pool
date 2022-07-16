#include "thread_pool.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <benchmark/benchmark.h>

constexpr auto NUM_TASKS = 10000;
std::vector<int> vNums(1000);

static void DoSetup(const benchmark::State& state) {
   std::iota(vNums.begin(), vNums.end(), 0);
   std::cout << "Setup...\n";
}

static void BM_Accumulate(benchmark::State& state) {
   thread_pool pool(state.range(0));
   std::cout << "Running with " << state.range(0) << " queues.\n";
   std::vector<std::future<int>> v;
   v.reserve(NUM_TASKS);
   for (auto _ : state) {
      for (int i = 0; i < NUM_TASKS; ++i)
         v.push_back(pool.async([]() { return std::accumulate(vNums.begin(), vNums.end(), 0); }));

      for (auto& f : v) {
         f.wait();
      }
   }
}

BENCHMARK(BM_Accumulate)->Threads(1)->Arg(1)->Arg(8)->Setup(DoSetup); //; 

BENCHMARK_MAIN();
