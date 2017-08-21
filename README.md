Collection of C librairies

The main goal of this repository is to improve my C coding skills
and beign able to have re-usable binaries that will save time when
building a new project.

All the project contains their own makefile, to enable quick build 
on any system. All the librairies are designed to be static. This
decision was driven by the desire to have portable executable. That will
make a final executable bigger in size but it will be able to run 
without worrying about runtime dependencies.

libnpmtoolkit
-------------------------------------
Provides a logger and a stringutils module. The logger follows the
same concept as the logger in Android. It is possible to set the 
verbosity level to control the amount of logs to be shown in the
console. The stringutils provides common functions to manipulate 
string, which can be cumbersome in C sometimes.

To build the libraires, go to the libnpmtoolkit folder in a console
and type:

root> make install

A 'dist' folder is created and you just need to import the .a and the
header file into your project.

libnpmnetwork
-------------------------------------
Provides a server implmentation that create a server socket and
start listening to a specific port on the host machine. This is 
all that server does. It needs a request handler (as a function pointer)
to handle any incoming client connection. This design will let any
developer to concentrate on the functionnality they want to offer 
instead of having a lot of boilerplate code just to start the server.

To build the librairies, go to the libnpmnetwork folder in a console
and type:

root> make install

A 'dist' folder is created and you just need to import the .a and the
header file into your project.

 
