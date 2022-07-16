#include "thread_pool.h"
#include <atomic>

thread_pool::thread_pool(std::size_t n) {
   m_nNumQueues = n == 0 ? NUM_THREADS : n;
   for (std::size_t i{}; i < m_nNumQueues; ++i)
      m_vQueueMutexes.emplace_back();

   m_vQueues.resize(m_nNumQueues);

#ifdef STATISTICS
   m_vOwnThreadFetch.resize(m_nNumQueues);
   m_vOtherThreadFetch.resize(m_nNumQueues);
#endif

   for (std::size_t i{}; i < NUM_THREADS; ++i) {
      m_vThreads.push_back(std::thread{ [this, i]() {
         std::size_t nIdx = i % m_nNumQueues;
         while (!m_bDone.load(std::memory_order_acquire)) {
            auto& slot = m_vQueues[nIdx];
            if (!slot.empty()) {
               if (auto& mutex = m_vQueueMutexes[nIdx]; mutex.try_lock()) {
                  std::lock_guard guard{ mutex, std::adopt_lock };

                  if (!slot.empty()) {
                     auto task = std::move(slot.back());
                     slot.pop_back();
                     task();

#ifdef STATISTICS
                     if (m_nNumQueues == NUM_THREADS) {
                        if (nIdx == i)
                           ++m_vOwnThreadFetch[i];
                        else
                           ++m_vOtherThreadFetch[i];
                     }
#endif
                     nIdx = i;

                     continue;
                  }
               }
            }

            nIdx = (nIdx + 1) % m_nNumQueues;
         }
      } });
   }
}

thread_pool::~thread_pool() {
   m_bDone.store(true, std::memory_order_release);

   for (auto& t : m_vThreads)
      t.join();

#ifdef STATISTICS
   std::size_t nTotalTasks{};
   if (m_nNumQueues == NUM_THREADS) {
      std::cout << "Stats:\n\n";
      std::size_t nIdx{};
      for (; nIdx < m_vOwnThreadFetch.size(); ++nIdx) {
         std::cout << "Thread " << nIdx << ": " << m_vOwnThreadFetch[nIdx] << "/" << m_vOtherThreadFetch[nIdx] << " fetched from own queue\n";
         nTotalTasks += m_vOwnThreadFetch[nIdx];
         nTotalTasks += m_vOtherThreadFetch[nIdx];
      }
      std::cout << "\nTptal tasks processed: " << nTotalTasks << "\n";
   }
#endif
}
