#pragma once
#include <arch/cpu/type.h>

namespace arch
{
   namespace tool
   {
      inline void relax_cpu(void)
      { //relax cpu
         asm volatile("pause;pause;pause;pause");
      }

      template <typename T = int32_t>
         class atomic_t
      {
         volatile T data;
      public:
         atomic_t(T data = 0) : data(data) {};
         
         T get(void) {return this->data;}
         void set(T data) {this->data = data;}

         void add(T data)
         {
            asm volatile
            (
               "lock;add %1,%0"
               : "=m" (this->data)
               : "r" (data)
            );
         }

         T add_and_get(T data)
         {
            T tmp = data;
            asm volatile
            (
               "lock;xadd %1,%0"
               : "=m" (this->data) , "+r" (data)
            );
            return data + tmp;
         }

         T set_and_get(T data)
         { //spinlock use this function
            asm volatile
            (
               "xchg %1,%0"
               : "=m" (this->data) , "+r" (data)
            );
            return data;
         }
      };
   }
}

