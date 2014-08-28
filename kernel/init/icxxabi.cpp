extern "C"
{

      //see http://wiki.osdev.org/C++ 
      //section Global objects 

   struct exit_function_entry
   {
      void (*destructor)(void *);
      void *object;
   };

   exit_function_entry __exit_functions[128];
   unsigned int __exit_function_count = 0;

   void *__dso_handle = 0;

   int __cxa_atexit(void (*f)(void *),void *object,void *)
   {  //register a destructor function
      if(__exit_function_count >= 
         sizeof(__exit_functions) / sizeof(__exit_functions[0]))
         return -1;

      __exit_functions[__exit_function_count++] =
         (exit_function_entry) {f,object};
      return 0;
   }

   void __cxa_finalize(void *f)
   {
      unsigned int i = __exit_function_count;
      if(!f)
      { //executable all destructor functions
         while(i--)
            if(__exit_functions[i].destructor)
               (*__exit_functions[i].destructor)(
                   __exit_functions[i].object);
         return;
      }
      while(i--)
      { //executable f as a destructor function
         if(__exit_functions[i].destructor == f)
         {
            (*__exit_functions[i].destructor)(
                  __exit_functions[i].object);
            __exit_functions[i].destructor = nullptr;
            //set the destructor to nullptr
         }
      }
   }
}
