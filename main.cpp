#include "thread_pool.h"
#include <iostream>

int main() {
   thread_pool pool{};

   std::vector<std::future<void>> v;
   v.reserve(1000);

   std::cout << "Pushing threads..." << std::endl;
   for (int i = 0; i < 1000; ++i)
      v.push_back(pool.async([i]() { std::cout << i << "\n"; }));

   std::cout << "Waiting for futures..." << std::endl;

   for (auto& f : v)
      f.wait();

   pool.shutDown();

   std::cout << "Done\n";


}
