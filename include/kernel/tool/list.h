#pragma once
#include <arch/cpu/type.h>


namespace kernel
{
   namespace tool
   {
      class list_t
      {
         list_t *previous;
         list_t *next;
public:
         list_t(list_t *list)
         {
            if(list)insert(list);
            //list_t list(0); just do nothing..
         }
         list_t(void)
         {
            this->next = this->previous = this;
            //init this list to a empty list
         }

         void insert(list_t *before)
         {
            list_t *n = before->next;
            n->previous = before->next = this;
            this->next = n;
            this->previous = before;
         }

         void insert_before(list_t *next)
         {
            list_t *before = next->previous;
            next->previous = before->next = this;
            this->next = next;
            this->previous = before;
         }

         void remove(void)
         {
            this->next->previous = this->previous;
            this->previous->next = this->next;
         }

         bool empty(void)
         {
            return this->next == this;
         }

         ~list_t(void)
         {
            //Need we "this->remove()" here?
         }

         //usage for example,
         //    list.value<abc_t,offset_of(abc_t,list)>();
         template <typename T,size_t offset>
            T value(void)
         {
            uint8_t *tmp = (uint8_t *)this->next;
            tmp -= offset;
            return (T)tmp;
         }
      };
   }
}
