#include "thread_pool.h"
#include <atomic>

thread_pool::thread_pool() {
   for (std::size_t i{}; i < NUM_THREADS; ++i)
      m_vQueueMutexes.emplace_back();

   m_vQueues.resize(NUM_THREADS);

   for (std::size_t i{}; i < NUM_THREADS; ++i) {
      m_vThreads.push_back(std::thread{ [this, i]() {
         std::cout << "Started thread " << i << std::endl;
         std::size_t nIdx = i;
         while (!m_done.load(std::memory_order_acquire)) {
            auto& slot = m_vQueues[nIdx];
            if (!slot.empty()) {
               if (auto& mutex = m_vQueueMutexes[nIdx]; mutex.try_lock()) {
                  std::lock_guard guard{ mutex, std::adopt_lock };

                  if (!slot.empty()) {
                     auto task = std::move(slot.back());
                     slot.pop_back();
                     task();

                     nIdx = i;

                     continue;
                  }
               }
            }

            nIdx = (nIdx + 1) % NUM_THREADS;
         }
         std::cout << "Got DONE signal\n";
      } });
   }
}

void thread_pool::shutDown() {
   m_done.store(true, std::memory_order_release);

   for (auto& t : m_vThreads)
      t.join();
}