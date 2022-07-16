#ifndef TASK_H
#define TASK_H

#include <memory>

class task {
   class concept_interface;

   std::unique_ptr<concept_interface> m_pContained;

public:

   template<typename Callable>
   task(Callable&& callable) : m_pContained{ std::make_unique<concept_impl<Callable>>(std::forward<Callable>(callable)) } {}

   task(const task& rhs) : m_pContained{ rhs.m_pContained->clone() } {}
   task& operator=(const task& rhs) {
      task copy{ rhs };
      swap(copy);
      return *this;
   }

   task(task&& rhs) noexcept {
      m_pContained = std::exchange(rhs.m_pContained, nullptr);
   }
   task& operator=(task&& rhs) noexcept {
      task copy{ std::move(rhs) };
      swap(copy);
      return *this;
   }
   ~task() = default;


   void operator()() { (*m_pContained)(); }

private:
   class concept_interface {
   public:
      virtual ~concept_interface() = default;
      virtual std::unique_ptr<concept_interface> clone() const = 0;
      virtual void operator()() = 0;
   };

   template<typename Callable>
   class concept_impl : public concept_interface {
   public:
      template<typename U = Callable>
      concept_impl(U&& callable) : m_callable{ std::forward<U>(callable) } {}
      std::unique_ptr<concept_interface> clone() const override { return std::make_unique<concept_impl>(m_callable); }
      void operator()() override { m_callable(); }

   private:
      Callable m_callable;
   };

private:
   void swap(task& rhs) noexcept {
      using std::swap;
      swap(m_pContained, rhs.m_pContained);
   }

   friend void swap(task& lhs, task& rhs) noexcept {
      lhs.swap(rhs);
   }
};


#endif
