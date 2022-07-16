#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <future>
#include <mutex>
#include <deque>
#include <vector>
#include <atomic>
#include <memory>
#include <iostream>
#include "task.h"


class thread_pool {
   using task_t = task;
   
   static inline const auto NUM_THREADS = std::thread::hardware_concurrency();

   std::vector<std::deque<task_t>> m_vQueues;
   std::deque<std::mutex> m_vQueueMutexes;
   std::vector<std::thread> m_vThreads;
   std::atomic<bool> m_bDone{ false };
   std::size_t m_nNumQueues{ NUM_THREADS };

#ifdef STATISTICS
   // statistics
   std::vector<std::size_t> m_vOwnThreadFetch, m_vOtherThreadFetch;
#endif

public:
   thread_pool(std::size_t n = 0);
   thread_pool(const thread_pool&) = delete;
   thread_pool& operator=(const thread_pool&) = delete;
   thread_pool(thread_pool&&) = delete;
   thread_pool& operator=(thread_pool&&) = delete;
   ~thread_pool();

   template<typename Func>
   auto async(Func&& f) -> std::future<decltype(f())> {
      using return_t = decltype(f());
      auto promise = std::promise<return_t>{};
      auto future = promise.get_future();
      if constexpr (std::is_same_v<return_t, void>) {
         _queue([p=std::move(promise), f=std::forward<Func>(f)] () mutable {
            f();
            p.set_value();
         });
      } else {
         _queue([p=std::move(promise), f=std::forward<Func>(f)] () mutable {
            p.set_value(f());
         });
      }
      return future;
   }

private:
   template<typename Task>
   void _queue(Task&& task) {
      static std::size_t nIdx{};
      while (true) {
         if (auto& m = m_vQueueMutexes.at(nIdx); m.try_lock()) {
            {
               std::lock_guard guard{ m, std::adopt_lock };

               m_vQueues.at(nIdx).emplace_front(std::forward<Task>(task));
            }
            nIdx = (nIdx + 1) % m_nNumQueues;

            break;
         }
         nIdx = (nIdx + 1) % m_nNumQueues;
      }
   }
};

#endif
