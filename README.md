# Client-Server-model

## Descripton
A client-server application in C++, and uses different communication fonctionnalities.
The client relies on sending a request to another program in order to access a service made available by a server. The server runs one or more programs that share resources with and distribute work among clients.
The client server relationship communicates in a request–response messaging pattern with the demon, and must adhere to a common communications protocol (defined in Architecture_schema.jpg), which formally defines the rules, language, and dialog patterns to be used. 

## Functionalities 
### Util.h
Contains all macro constants, data structures, functions
allowing the management of errors as well as the function allowing
construct the names of the pipes.
### client.c
the main function
1. Allocation and initialization of two character strings containing the name of the
input and output pipes.
2. Addition of certain signals in the set of received signals.
3. Creation and projection of the shared memory segment.
4. Creation and initialization of a request.
5. Then the customer gets the message in one of the possible ways.
6. If the queue is not full, we put a lock to write the request before
unlocking.
7. We open the tubes for reading and writing.
8. Then we create a thread which will launch our writing function.
9. The client reads the data sent by the daemon on the input pipe and displays
on the terminal.
10.Finally, we close the descriptors and then free the resources before
to leave.
the manager
Exits the application via the SIGINT, SIGHUP, SIGTERM, SIGTSTP and
SIGPIPE
the start function
Writes those on standard input to the write pipe.
the destroy function
Releases the program's resources and cancels the current thread.  
### Deamon.c
the main function
1. Allocation of a shared memory segment.
2. Projection and initialization of our fifo file.
3. Initialization of semaphores.
4. Construction of the set of signals.
5. Then we enter an infinite loop which creates a thread for each request
in the File.
6. Finally, when the daemon is stopped it frees the resources
allocated. 

## Built in
Unix system- GNU Compiler 

## Author
* **HARAKI YOUNESS** - (https://github.com/Codeharaki)
