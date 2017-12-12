Collection of C librairies

The main goal of this repository is to improve my C coding skills
and beign able to have re-usable binaries that will save time when
building a new project.

All the project contains their own makefile, to enable quick build 
on any system. All the librairies are designed to be static. This
decision was driven by the desire to have portable executable. That will
make a final executable bigger in size but it will be able to run 
without worrying about runtime dependencies.

The other way to use this library is to copy the header and source
file directly into your project. Effort has been done to reduce 
the dependencies between different source files.

crypto
-------------------------------------
Based on the LibTomCrypt cryptographic library, this package contains a
simplified code base. The goal is to let the developer choose the 
encryption algorithms that it only needs. Just copy the header file
with the corresponding source file and compile it with the target
project.

This library is ideal when all the source code needs to be included
at compilation time and not having a distinct static library for crypto
operation.

Ciphers
- Advanced Encryption Standard (ECB, CBC modes)

Digest
- MD5
- SHA1
- SHA256
- SHA512

Pseudo-Random Number Generation
- Fortuna


To build the libraires, go to the crypto folder in a console
and type:

> make clean; make build

Copy the toolkit.a file with the header into your project.


toolkit
-------------------------------------
Provides general purpose code to speed up development. The following
component are available:

FileUtils
- Save/Load data structures to/from files.
- Many utils function like read file content, get full path name, etc...

Logger
- Logger that produces useful log entry with timestamp and tag. Most be used with server application!

Padding
- Enables any content to be padded to fit a certain block size. Unpadding function also available.

Conversion
- Convert a hexadecimal string into a raw byte array.


To build the libraires, go to the toolkit folder in a console
and type:

> make clean; make build

Copy the toolkit.a file with the header into your project.


network
-------------------------------------
Provides a server implmentation that create a server socket and
start listening to a specific port on the host machine. This is 
all that server does. It needs a request handler (as a function pointer)
to handle any incoming client connection. This design will let any
developer to concentrate on the functionnality they want to offer 
instead of having a lot of boilerplate code just to start the server.

To build the librairies, go to the network folder in a console
and type:

> make clean; make build

Copy the toolkit.a file with the header into your project.

 
