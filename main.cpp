#include "thread_pool.h"
#include <iostream>

int main() {
   thread_pool pool{};

   std::vector<std::future<int>> v;
   v.reserve(1000);

   std::cout << "Pushing threads..." << std::endl;
   for (int i = 0; i < 1000; ++i)
      v.push_back(pool.async([i]() { return i; }));

   std::cout << "Waiting for futures..." << std::endl;

   for (auto& f : v) {
      f.wait();
      std::cout << f.get() << " ";
   }

   pool.shutDown();

   std::cout << "\n\nDone\n";


}
