#pragma once
#include <arch/tool/atomic.h>
#include <kernel/compiler.h>

namespace kernel
{
   namespace tool
   {
      class spinlock_t
      {
         arch::tool::atomic_t<uint8_t> atomic;
public:
         spinlock_t(void) : atomic(0) {};

         void lock(void)
         {
            if(likely(atomic.set_and_get(1) == 0))
               return;

            while(atomic.set_and_get(1) == 1)
               arch::tool::relax_cpu();
         }

         bool try_lock(void)
         {
            return !atomic.set_and_get(1);
         }

         void unlock(void)
         {
            atomic.set(0);
         }
      };

      class spinlock_locker
      { //auto unlock at the destructor...
         bool locked;
         spinlock_t *_lock;
public:
         spinlock_locker(spinlock_t *lock,bool locked = false)
         {this->_lock = lock;this->locked = locked;}

         void lock(void) {_lock->lock(); locked = true;}
         void unlock(void) {_lock->unlock(); locked = false;}

         ~spinlock_locker(void) {if(locked) _lock->unlock();}
      };
   }
}
