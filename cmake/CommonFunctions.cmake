##############################################
#
# CMake project build suite v1.1.0
# Author: Oleg F., fedorov.ftf@gmail.com
#
##############################################

function(add_prefix list prefix)
   set(listvar
   	)
   foreach(item ${ARGN})
    	list(APPEND listvar "${prefix}/${item}")
   endforeach()
   set(${list} "${listvar}" PARENT_SCOPE)
endfunction(add_prefix)