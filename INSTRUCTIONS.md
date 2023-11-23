
# Assignment 4 - Socket Communication

Network sockets can be a powerful way to implement Inter-Process communication, even between processes that don't reside on the same machine!

In this assignment, you'll practice network sockets by writing a chat application that can send and receive messages with another instance of itself.

This assignment is to be completed individually.

## Network Sockets

You will use ***UDP*** network sockets to send and receive all data. There are many tutorials available online which may help your learning (always remember to ***study*** other people's code, but never directly copy or plagiarize).

You may find the following functions and documentation useful:

* Create a socket with [socket](https://linux.die.net/man/2/socket)

* Bind a listening socket with [bind](https://linux.die.net/man/2/bind)

* Send data with [send or sendto](https://linux.die.net/man/2/send)

* Receive data with [recv](https://linux.die.net/man/2/recv)

* Close a socket with [close](https://linux.die.net/man/2/close)

Some of the code from above links uses *TCP* sockets rather than ***UDP***. Make sure all your sockets are of type `SOCK_DGRAM`. You may find the following tutorial helpful, which may or may not actually work in {the current year}:

* [C++ Implementation of a UDP Client/Server](https://linux.m2osw.com/c-implementation-udp-clientserver)

## Sequence of Events

You will launch two instances of your program, in order to simulate two chat partners.

Your program will expect three command line arguments upon launch:

1. The number of a port, which the program will use to listen for incoming data

2. A hostname that the program will use to send outgoing data

3. Another port, which the program will use as a destination for outgoing data

    * Using a different port for outgoing and incoming data will allow you to launch two instances of your program on the same machine. If we only used one port, you'd have to run the second instance on a different computer or VM.

After parsing and storing these arguments, your program will initialize two network sockets:

* One for sending data

* One for receiving data

Your program will then spawn two threads:

* One to take user input and send it as network data to the other program instance (i.e., your "chat partner")

* One to receive network data from the other program instance, and print it to the console

After both threads have been spawned, your main thread will attempt to join each thread, then exit.

### Sender Thread

The sender thread prompts the user to enter a line of input (hint: [std::getline](https://en.cppreference.com/w/cpp/string/basic_string/getline)), which blocks until the user presses enter. Once the user's input has been received, the sender thread uses its socket to send the chat message (including a [null terminator](http://www.cs.ecu.edu/karl/2530/spr17/Notes/C/String/nullterm.html)) to its chat partner. Once the data is sent, the sender thread then repeats itself by asking the user for another line of input.

If the user enters `q` or `Q` as input, the sender thread considers the session closed, and will take steps to end the chat session and exit the program.

When the sender thread decides to terminate the chat session, it first sends a special sequence of characters to the chat partner, three times in a row, to let the partner know the chat session is ending.

The sender thread will watch a special variable `Chat::quitting_` and exit its loop when it evaluates to `true`.

More information can be found in code comments.

### Receiver Thread

The receiver thread repeatedly tries to receive data on the listening socket. Whenever a full chat message has been received, the receiver thread prints it to the terminal.

Sometimes it is possible that the receiver thread only receives a partial chat message, with no [null terminator](http://www.cs.ecu.edu/karl/2530/spr17/Notes/C/String/nullterm.html)). When this happens, the receiver thread simply keeps track of all the message fragments it has received, and concatenates them together. Eventually, the receiver will see a null terminator and know the message is complete. At that point, it prints the full message to the terminal.

If the receiver thread receives the special "quit" sequence of characters (more explained in code comments) that indicate the end of the chat session, it will send that sequence back to the partner three times, then take steps to end the chat session and end the program.

The sender thread will also watch the special variable `Chat::quitting_` and exit its loop when it evaluates to `true`.

Again, more information can be found in code comments.

## Files to Edit

This section describes which files you should edit for your submission. The easiest thing to do is check the comments inside each source file, but here is a summary.

The *main.cpp* file should not be edited. It launches your `Chat` program for you.

All your work will go into *Chat.hpp* and *Chat.cpp*. Do not remove existing functions from those files, nor modify existing function prototypes. You may, however, *add* new helper functions to the *Chat.cpp* file, as long as you remember to add the corresponding prototype in *Chat.hpp*.

The following files ***should not be edited***:

* INSTRUCTIONS.md
* CPP_Tests.cpp
* Makefile
* main.cpp

## Hints and Tips

This section contains hints and tips to help you along.

### Printing Error Strings

Sometimes, using `errno` and `perror` can be a pain, especially if you want to format a nice `std::string`. Instead, consider using [strerror](https://www.cplusplus.com/reference/cstring/strerror/).

### Flushing Terminal Output

You may have trouble passing all tests, or seeing all output if you're not properly flushing `std::cout`. Many students like to end lines with the `\n` character. However, this may sometimes cause your program to withhold output from the terminal until a later time. Using `std::endl` can solve this problem:

```cpp

//	Not great
std::cout << "Hello!\n";

//	Good!
std::cout << "Hello!" << std::endl;

```

### Finding a Sequence in an std::string

You're probably going to use `std::string` instances instead of `char` arrays for the most part, because they're easier to work with. But how will you detect whether a sequence of characters (such as `Chat::QUIT_SEQUENCE`) appears inside an `std::string`?

You can simply call on [std::string::find](https://www.cplusplus.com/reference/string/string/find/) with the sequence in question. It will return the position of the first matching character (of the sequence), or [std::string::npos](https://www.cplusplus.com/reference/string/string/npos/) if the sequence is not found.

### Quitting the Chat

Regardless of which partner ends the chat (by sending the `Chat::QUIT_SEQUENCE`), *both* partners should end up sending the `Chat::QUIT_SEQUENCE`. This is because both partners will usually have a receiver thread that is currently blocking on a call to `recv`. Having a final message sent both ways during disconnection is a convenient way to break the block. You don't have to worry about an infinite loop going back and forth between both partners, because each partner should `break` from their receive loop and exit their receive thread, after seeing the quit sequence.

Regardless, the sender thread will be blocked waiting for user input, so the easiest solution there is to have the receiver thread simply print a message instructing the user to press enter.

### Testing Against netcat

Linux has a nifty tool called *netcat*, which usually appears as *nc* on Ubuntu systems. You can tell the *nc* program to listen on a port and print any data it receives. Specifically, you may with to install the package `netcat-openbsd` in Ubuntu.

For example, suppose you'd like to check whether your chat program is sending any data. That might be hard to do if you also don't know whether your chat program is receiving any data! We know *netcat* works, so we could try something like the following:

```console
$ nc -u -l -k 8823
```

With the above, *netcat* will listen indefinitely for connections, and print anything sent to it. You can do something similar to check that your program can receive connections:

```console
$ echo -ne 'Hello!\0' | nc -u 127.0.0.1 8822
```

Note the null terminator present in the above command, which you'd have trouble simulating typing manually.

More information can be found with the commands `man nc` and `man echo`.

***Warning:*** Do not attempt to subvert the spirit of this assignment by using calls to *netcat* in your code. You will receive a zero. Use sockets instead.

### Testing Against a Second Instance

An extra Makefile target has been added to this project: `run2`. It essentially runs your program but with the ports reversed, which will allow you to create two instances that communicate with each other. Try opening two terminals.

In the first terminal, run:

```console
$ make run
```

Then, in the second terminal, run:

```console
$ make run2
```

### Testing Against a Second ... Computer!

If you're feeling adventurous, you can manually run your program (which will allow you to specify ports and the remote hostname) on two separate machines!

For example, suppose you have two computers on your local network. The first has the IPv4 address *192.168.1.21* and the second has *192.168.1.27*.

Open a terminal on the first computer and run:

```console
$ ./main 8822 192.168.1.27 8823
```

Then, open a terminal on the second computer and run:

```console
$ ./main 8823 192.168.1.21 8822
```

(note the reversal of ports)

If your program works correctly, you may have just successfully tested your first network-capable application!

### When a Socket Refuses to Quit

Sometimes, you may notice your program exits, but doesn't release the listening socket. When this happens, a new run of your program may fail because it is trying to listen on a port that is still in use.

Use the following command to see if your computer is still listening on any ports:

```console
$ ss --udp --listening --numeric
```

You can also try to find any leftover orphan threads with *htop*:
```console
$ htop
```

*Hint*: Use F3 to search for processes by string. Yours probably has the word "main" somewhere, or perhaps "127.0.0.1". Once you locate the process, hit "k" then "enter", to kill it.

### Concatenating Message Fragments

Sometimes, a call to `recv` might return a partial message. This can be a problem when trying to concatenate strings together, because string functions tend to rely on the presence of null terminators. To solve this issue, you may want to artificially tack on a null terminator to the end of any message fragments you encounter, just so you can concatenate them together into a whole message.

For example, suppose your chat partner sends the message `"Hello\0"`. Recall we can use `\0` to represent a null terminator inside a string. If the message is sent as two fragments, your `recv` call will end up receiving the following strings:

1. `"He"`
2. `"llo\0"`

Notice the absence of a null terminator in the first fragment! This would make string manipulation, searching, concatenation, and other operations somewhat difficult to perform with standard functions. When using a receive loop, we would typically call `recv` with a temporary buffer and copy each fragment received to a somewhat more permanent buffer/string, until we encounter the null terminator in the second fragment. But string functions probably won't work well if the buffer simply receives `"He"`. Therefore, it might be wise to add a null terminator to `"He"` before copying it to the more permanent string. Think of the following steps:

1. `recv` gives us `"He"` (and reports a received length of 2).

2. We add a null terminator to the end of the buffer (at index 2).

3. The buffer now contains `"He\0"`, which is easier to send to our more permanent buffer.

4. `recv` then gives us `"llo\0"` (and reports a received length of 4).

5. Since we already see a null terminator in this fragment, there is no need to add on a null terminator

6. We can easily concatenate `"llo\0"` onto `"He\0"`, giving us `"Hello\0"`.

## Sample Output

Below is sample output from a complete program. Yours might not need to look exactly the same, as long as you pass your unit tests:

```text

$ make run1
g++ -Wall -pedantic -g -c -std=c++17 main.cpp -o main.o
g++ -Wall -pedantic -g -c -std=c++17 Chat.cpp -o Chat.o
g++ -Wall -pedantic -g -std=c++17 main.o Chat.o -o main
./main "8822" "127.0.0.1" "8823"
Enter message (or q to quit) > Hey dude. What's up? 
Enter message (or q to quit) > 
Buddy > Honestly, I just need to eat these cookies...

Buddy > You mind if we talk later?
Yeah that's fine. Talk to you later!
Enter message (or q to quit) > qqqqqq
Enter message (or q to quit) > q

Sending quit!

*** Buddy has disconnected ***
(press enter to exit if stuck)
Program exiting ...
```

Notice that when your chat partner sends a message, it pushes your `cin` prompt down a few lines. That's okay. If you wanted to, you could probably upgrade your program to reprint the prompt after receiving your partner's chat message.

## Compilation, Execution, Submission

As with the previous assignment, you'll push your code to Github whenever you make significant progress. You'll also compile/execute/test your program using [GNU Make](https://www.gnu.org/software/make/) (try `make help` for commands). Finally, you'll submit your code for grading to Gradescope.


## Copyright and Acknowledgment

Copyright 2022, Mike Peralta.

This assignment prompt was inspired by a prompt found {? somewhere ?}.

This prompt and starter code are licensed via: [Creative Commons; Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)](https://creativecommons.org/licenses/by-sa/4.0/).

![CC By-SA 4.0](images/CC_BY-SA_4.0.png "CC By-SA 4.0")





