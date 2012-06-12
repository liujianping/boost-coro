boost coroutine another version(0.1)
contact: rabbit.ljp@gmail.com

1. installation
   
   get the source from the github(https://github.com/liujianping/boost-coro)

   $: cd boost-coro
   $: mkdir build
   $: cd build  
   $: make 
   $: make install
      
   boost-coro default install path will be at the /opt/local, 
   and only install static lib and header files. if you need dynamic lib, modify
   the CMakeLists.txt.

   examples:
   
   if you want to test the examples, uncomment the CMakeLists.txt's subdirectory line
for the examples, then rebuild the project.

2. bugs

   the current version is a draft version, just a beginning version 0.1; 
   must have some bugs, you can fixed the bugs yourself or report the bugs by sent
   email to me with the address: rabbit.ljp@gmail.com or leave a message at my 
   homepage: http://tiemaile.com
    
   and you can read some design & implemention ideas from the web: 
   http://tiemaile.com.

   any suggestion & message would be welcome!