# Simple Talk (S-talk)
Multi-threaded program for bidirectional communication between different terminals on the same network. Uses UNIX UDP, IPC, and the client/server model. 

## How to Run
```
# Clean and build executable
$ make
# Output "s-talk" will be produced. Example to run:
$ ./s-talk 54321 localhost 12345 
# Then on another terminal run:
$ ./s-talk 12345 localhost 54321 
# Enter <!> to exit
```

## Features 
- Can send messages using the CAT command from file
- Sending <!> ends session on both terminals

## Tests
- Tests in "Tests" directory
- Cat txt files into the terminal