#include "thread_pool.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <benchmark/benchmark.h>

static constexpr auto NUM_TASKS = 10000;

class MyBenchmark : public benchmark::Fixture {
public:
   std::vector<int> vNums;
   static inline bool init = false;

   void SetUp(const benchmark::State& state) BENCHMARK_OVERRIDE {
      vNums.resize(1000);
      if (not init) {
         std::iota(vNums.begin(), vNums.end(), 0);
         std::cout << "Setup...\n";
         init = true;
      }
   }
};

BENCHMARK_DEFINE_F(MyBenchmark, AccumulateTest)(benchmark::State& state) {
   thread_pool pool(state.range(0));
   std::cout << "Running with " << state.range(0) << " queues.\n";
   std::vector<std::future<int>> v;
   v.reserve(NUM_TASKS);
   for (auto _ : state) {
      for (int i = 0; i < NUM_TASKS; ++i)
         v.push_back(pool.async([this]() { return std::accumulate(vNums.begin(), vNums.end(), 0); }));

      for (auto& f : v) {
         f.wait();
      }
      v.clear();
   }
}

BENCHMARK_REGISTER_F(MyBenchmark, AccumulateTest)->Threads(1)->Arg(1)->Arg(8); //; 

BENCHMARK_MAIN();

