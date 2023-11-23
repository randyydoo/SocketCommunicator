
/**
 *
 * Most of your work goes here.
 *
 * Use this file to implement the functions declared in Chat.hpp
 *
 * Do not place implementation into Chat.hpp; That's what this file is for.
 *
 */

//
#include "./Chat.hpp"

//

//	TODO: More includes?
#include <iostream>
#include <unistd.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdexcept>

using std::cout, std::cin, std::endl;

///	Main namespace for this class
namespace CPSC351
{
	/*************************************************************
	 * Begin non-member functions, or regular "global" functions *
	 *************************************************************/

	/**
	 *
	 * Non-member function (regular function) that will
	 * 	serve as an entry point for the sender thread.
	 *
	 * Once inside this function, re-cast 'param' to a 'Chat*'
	 * 	and use it to call on a member function for your Chat
	 * 	instance.
	 *
	 * This is a pattern sometimes used to wrap C++ class objects
	 * 	around C functions.
	 *
	 * This function has been completed for you.
	 *
	 * You must complete c_receiverThread on your own
	 *
	 * (your welcome)
	 *
	 */
	void *c_senderThread(void *param)
	{
		//	Grab the 'this' pointer, so we can get back inside our instance
		Chat *this_ = static_cast<Chat *>(param);
		this_->senderThread();
		return NULL;
	}

	/**
	 * Same idea as c_senderThread, but this should call
	 * 	the Chat::receiverThread() method.
	 */
	void *c_receiverThread(void *param)
	{
		Chat *this_ = static_cast<Chat *>(param);
		this_->receiverThread();
		return NULL;
	}

	/***********************************************************
	 * End non-member functions, or regular "global" functions *
	 ***********************************************************/

	/**
	 * Constructor
	 *
	 * Remember the listening port, the outgoing hostname
	 * 	and the outgoing port.
	 */
	Chat::Chat(int port_listen, std::string hostname, int port_out)
	{
		port_listen_ = port_listen;
		hostname_ = hostname;
		port_out_ = port_out;
	}

	/**
	 * Destructor
	 *
	 * Perform appropriate cleanup operations here, if any
	 *
	 * Try closing the sockets here
	 */
	Chat::~Chat()
	{
		close(socket_listen_);
		close(socket_out_);
	}

	/**
	 * The run() function starts the chat session with these basic steps:
	 *
	 * 1. Call the two functions that create sockets.
	 *
	 * 2. Call this->spawnThreads(),
	 *      which will in turn spawn the sender and receiver threads,
	 *      and save the thread IDs to member variables.
	 *
	 * 3. Call this->joinThreads(), which will join the threads.
	 *
	 */
	void Chat::run()
	{
		initListeningSocket();
		initOutboundSocket();
	/*	socket_listen_ = socket(AF_INET, SOCK_DGRAM, 0);
		if (socket_listen_ == -1) {
			cout <<"Error: Socket listen failed!"<<endl;
			throw 1;
		}
		socket_out_ = socket(AF_INET, SOCK_DGRAM, 0);
		if (socket_out_ == -1) {
			cout <<"Error: Socket out failed!"<<endl;
			throw 1;
		}
		*/
		spawnThreads();
		joinThreads();
	}

	/**
	 *
	 * Return true if the listening socket is "valid"; false otherwise
	 *
	 * Probably want to rely on isSocketValid() to avoid repeating code
	 *
	 */
	bool Chat::isListeningSocketValid() const
	{
		return isSocketValid(socket_listen_);
	}

	/**
	 * Initialize/create the listening socket that will receive data
	 *   from your chat partner.
	 *
	 * You'll want to do the following things:
	 *
	 * 1. Close the listening socket by calling closeListeningSocket(),
	 *      just in case its already open.
	 *
	 * 2. Create and configure a sockaddr_in struct that listens
	 *      on the UDP port assigned to this->port_listen_.
	 *
	 *    You probably want to use a bind address of INADDR_ANY,
	 *      so the socket can listen on all IP addresses for your computer.
	 *
	 * 3. Create an actual UDP datagram internet socket.
	 *
	 * 4. Throw a runtime error if the socket fails to create
	 *
	 * 5. Bind the socket so the sockaddr_in struct's information
	 *
	 * 6. Throw a runtime error if the socket fails to bind
	 *
	 * 7. Remember the created socket as a member variable
	 */
	void Chat::initListeningSocket()
	{
		closeListeningSocket();
		int result;
		
		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_port = htons(port_listen_);

		//char* host = &hostname_[0];
		//inet_aton("127.0.0.1",&address.sin_addr);
	
		address.sin_addr.s_addr=INADDR_ANY;
		memset(&(address.sin_zero),0,8);

		socket_listen_ = socket(AF_INET,SOCK_DGRAM,0);
		if (socket_listen_ == -1){
			close(socket_listen_);
			throw("Socket_out error!");
		}
		result=bind(socket_listen_, (struct sockaddr *) &address, sizeof(address));

		if (result<0){
			cout <<"Error: Socket fail to bind!"<<endl;
			throw 1;
		} 
	}

	/**
	 * Simply return the listening socket
	 */
	int Chat::getListeningSocket() const
	{
		return socket_listen_;	
	}

	/**
	 *
	 * Close the listening socket, if it's currently valid.
	 *
	 * Probably want to rely on closeSocket() to avoid repeating code
	 *
	 */
	void Chat::closeListeningSocket()
	{
		if (isListeningSocketValid()){
			closeSocket(socket_listen_);
		}
	}

	/**
	 *
	 * Return true if the outbound socket is "valid"; false otherwise
	 *
	 * Probably want to rely on isSocketValid() to avoid repeating code
	 *
	 */
	bool Chat::isOutboundSocketValid() const
	{
		if (isSocketValid(socket_out_)){
			return true;
		}else{
			return false;
		}
	}

	/**
	 * Initialize/Create the outbound socket that will send data
	 *   to your chat parter.
	 *
	 * You'll want to do the following:
	 *
	 * 1. Call closeOutboundSocket(), just in case it's already valid
	 *
	 * 2. Create a UDP datagram internet socket
	 *
	 * 3. Throw a runtime error if the socket fails to create
	 *
	 * 4. Remember the socket as a member variable
	 */
	void Chat::initOutboundSocket()
	{
		closeOutboundSocket();

		socket_out_=socket(AF_INET, SOCK_DGRAM, 0);

		if (socket_out_ == -1){
			cout << "Socket_out error!"<<endl;
			throw 1;
		}
	}

	/**
	 * Simply return the outbound socket
	 */
	int Chat::getOutboundSocket() const
	{
		return socket_out_;
	}

	/**
	 *
	 * Close the outbound socket, if it's currently valid.
	 *
	 * Probably want to rely on closeSocket() to avoid repeating code
	 *
	 */
	void Chat::closeOutboundSocket()
	{
		if (isSocketValid(socket_out_)){
			closeSocket(socket_out_);
		}
	}

	/**
	 * Handle input that the user has entered
	 *
	 * 1. If the user has entered the string "q" or "Q",
	 *      print the string "Sending quit!" and
	 *      call on this->quit()
	 *
	 * 2. If the user has entered anything else,
	 *      send the input to a call to this->sendChatMessage()
	 */
	void Chat::handleUserInput(std::string input)
	{
		if (input == "q" || input == "Q")
		{
			cout << endl<<"Sending quit!" <<endl<<endl;;
			this->quit();
		}
		else
		{
			this->sendChatMessage(input);
		}
		return;
	}

	/**
	 * Send a chat message!
	 *
	 * 1. Return immediately if the message is empty
	 *
	 * 2. Throw a runtime error if the outbound isn't valid
	 *
	 * 3. Create a sockaddr_in struct and fill it with data pointing to
	 * 	the outbound hostname and outgoing port
	 *
	 * 4. Send the data with sendto()
	 *
	 * 5. If no bytes were sent, throw a runtime error
	 *
	 * 6. Otherwise you're done
	 *
	 * You would normally want to send your data in a loop,
	 * 	piece by piece (checking the return value of sendto) until
	 *  its done. But for the purposes of this project, we can
	 * 	probably just assume one call to sendto() ends up sending
	 * 	the entire chat message.
	 */
	void Chat::sendChatMessage(std::string message)
	{
		const int length = message.length();
 
    	// declaring character array (+1 for null terminator)
    	char* char_array = new char[length + 1];
 
    	// copying the contents of the
    	// string to char array
    	strcpy(char_array, message.c_str());

		
		if (message.empty())
		{
			return;
		}
		if(isOutboundSocketValid()==false){
			cout <<"Error: Outbound is invalid!"<<endl;
			throw 1;
		}
		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_port = htons(port_out_);

		//char* host = &hostname_[0];
		//inet_aton(host, &address.sin_addr);
		//memset(address.sin_zero,0,8);
		address.sin_addr.s_addr=INADDR_ANY;

		socket_out_ = socket(AF_INET,SOCK_DGRAM,0);
		if (socket_out_ == -1){
			cout <<"Error: Socket out fail to create!"<<endl;
			throw 1;
		}

/*		if(sendto(socket_out_, char_array,length, 0, (struct sockaddr *) &address, sizeof(address))==-1){
    		perror("TTFTPERROR: sendto() failed to send ack 0");
    		throw 1;
		};
		*/
		int result;
		result=sendto(socket_out_, char_array,length, 0, (struct sockaddr *) &address, sizeof(address));
		if (result<1){
			cout <<"Error: Sending error! "<<result<<endl;
			throw 1;
		}
	}

	/**
	 * Receive a chat message
	 *
	 * Start by throwing a runtime error if the listening socket isn't valid
	 *
	 * In this program, each chat message should end with a null terminator.
	 * It is possible that one call to recv() might only give you part of a messsage,
	 * 	and not have a null terminator at the end. Thus, you need to figure out
	 * 	a way to call recv() in a loop, and concatenate characters received
	 * 	until you see the null terminator.
	 * Once you see the null terminator, return the entire concatenated string.
	 *
	 * Here is an idea:
	 *
	 * 	1. Start with an empty string to represent the message you have received
	 *
	 *  2. Enter into a while loop
	 *
	 *  2a. Try to receive some data from the listening socket
	 *
	 * 	2b. If the received length is -1, pretend you have received "SOCKETERROR" and break the loop
	 *
	 * 	2c. If the data you have received doesn't contain a null terminator at the end,
	 * 		add one artificially so you can concatenate it onto your message string
	 *
	 * 	2d. If the data you received contains a null terminator at the end,
	 * 		this is the end of the current chat message. Add the piece you've just
	 * 		received to the message string and break the loop.
	 *
	 *  4. Return the message you have received.
	 *
	 * Note that this function uses a call to recv() with no
	 * 	consideration for timing out or select(), which means the
	 * 	thread could get stuck forever if your partner were to disconnect.
	 *
	 * This is why - when someone ends the chat session - we have BOTH
	 * 	partners send the quit sequence.
	 */
	std::string Chat::receiveChatMessage()
	{
		char mess[BUFFER_SIZE];
		std::string str="";
		struct sockaddr_in addr;
		
		memset(&addr, 0 , sizeof(addr));
		socklen_t length = sizeof(addr);
		
		if(isListeningSocketValid()==false){
			cout <<"Error: invalid listen socket "<<endl;
			throw 1;
		}

		while (true){
			int result=recvfrom(this->socket_listen_,(char*)mess, BUFFER_SIZE,MSG_WAITALL, (struct sockaddr*)&addr,&length);
			if(mess[result - 1] == '\0'){	//check null terminator at the end
				return str + mess;
	}		else{
			mess[result] = '\0';
			str = str +mess;}
	}
			return str;
	}
		
	/**
	 * Do whatever needs to be done based on the chat message
	 * 	you have just received from your chat partner
	 *
	 * If you've detected that the Chat::QUIT_SEQUENCE appears
	 * 	in *any part* of the received chat message,
	 *  print "*** Buddy has disconnected ***" on a line,
	 * 	then print "(press enter to exit if stuck)" on
	 *  another line so the user knows how to break
	 * 	the std::getline() call. Then, call this->quit() to
	 * 	perform quitstuffs
	 *
	 * Otherwise, print "Buddy > " and then the received
	 * 	chat message.
	 */
	void Chat::handleReceivedChatMessage(std::string message)
	{
		if(message.find(QUIT_SEQUENCE) != std::string::npos){
			cout << "*** Buddy has disconnected ***"<< endl;
			cout << "press enter to exit if struck" << endl;
			this->quit();
		}else{
			cout << endl<< "Buddy >" << message <<endl;
		}
	}

	/**
	 * Properly quit the chat session
	 *
	 * 1. Set this->quitting_ to true, so loops elsewhere know to break
	 * 		(don't worry about race conditions; this app is simple)
	 *
	 * 2. Send Chat::QUIT_SEQUENCE as a message to your partner
	 * 		THREE TIMES, so they have a good chance to exit their loop, too.
	 */
	void Chat::quit()
	{
		this->quitting_=true;
		//this->isQuitting();
		for(int i=1;i<=3;i++){
			//cout <<QUIT_SEQUENCE<<endl;
			this->sendChatMessage(this->QUIT_SEQUENCE);
			
			//exit(0);
		}
		//return;
	}

	///	Return true if the chat app wants to quit; False otherwise
	bool Chat::isQuitting()
	{
		if(this->quitting_== true){
			return true;
		}else{
			return false;
		}
	}

	/**
	 * Return true if the socket is valid, false otherwise
	 *
	 * The most proper way to detect a valid socket
	 * 	is by calling getsockopt() and checking the return value.
	 *
	 * However, for simplicity, we can just say
	 * 	that a "valid" socket will have an integer value of at least 1
	 */
	bool Chat::isSocketValid(int sock) const
	{
		if (sock >= 1)
		{
			return true;
		}
		return false;
	}

	/**
	 * Close a socket
	 *
	 * Notice this takes a socket by REFERENCE
	 *
	 * This means you can call on it to close either of your sockets
	 * 	and expect it will properly modify the correct member variable
	 *
	 * 1. If the socket isn't valid, return immediately
	 *
	 * 2. close() the socket
	 *
	 * 3. Set the socket's value to Chat::INVALID_SOCKET,
	 * 		which is just -1
	 */
	void Chat::closeSocket(int &sock)
	{
		if (isSocketValid(sock) == false)
		{
			return;
		}
		close(sock);
		sock = INVALID_SOCKET;
	}

	/**
	 * Spawn the sender and receiver threads.
	 *
	 * The sending thread should enter into c_senderThread(),
	 * 	which in turn will call the current instance's Chat::senderThread()
	 *
	 * The receiving thread should enter into c_receiverThread(),
	 * 	which in turn will call the current instance's Chat::receiverThread()
	 *
	 * You'll also want to remember thread IDs with member variables.
	 *
	 * Don't forget to check for errors, and throw when unsuccessful
	 */
	void Chat::spawnThreads()
	{
		int result;
		result= pthread_create(&tid_sender_,NULL, c_senderThread,this);

		if (result<0){
			cout << "c_senderThread created error!" << std::endl;
			throw 1;
		}
		result= pthread_create(&tid_receiver_,NULL, c_receiverThread,this);
		if (result<0){
			cout << "c_receiverThread created error!" << std::endl;
			throw 1;
		}
		return;
	}

	/**
	 * senderThread() is just a loop that calls on two other
	 * 	functions to handle user input.
	 *
	 * 1. Enter into a loop.
	 *
	 * 2. Grab an std::string of the user's input by calling
	 *      on the member function getUserInput().
	 *
	 * 3. Call on the member function handleUserInput()
	 *      to actually handle the user's input.
	 *
	 * 4. Break the loop if the member variable this->quitting_ is true.
	 *
	 * 5. Otherwise, keep looping.
	 *
	 */
	void Chat::senderThread()
	{
		std::string str;
		while (quitting_==false){
			 str= getUserInput();
			handleUserInput(str);
		}
			
		//return;
	}

	/**
	 * receiverThread() is just a loop that continually tries to receive
	 * data from your chat partner.
	 *
	 * 1. Enter into a loop.
	 *
	 * 2. Call this->receiveChatMessage() to get a string from
	 *      your chat partner.
	 *
	 * 3. Call on this->handleReceivedChatMessage() with the string.
	 *
	 * 4. Break the loop if the member variable this->quitting_ is true.
	 *
	 * 5. Otherwise, keep looping.
	 *
	 */
	void Chat::receiverThread()
	{
		std::string msg="";
		while(this->quitting_==false){
			msg=this->receiveChatMessage();
			this->handleReceivedChatMessage(msg);
		}
	}

	/**
	 * Get user input from the keyboard and return as an std::string
	 *
	 * 1. Prompt the user for input with:
	 *    "Enter message (or q to quit) > "
	 *
	 * 2. Use std::getline (or similar) to fetch an entire line
	 *      of user input.
	 *
	 * 3. Return the line as an std::string
	 */
	std::string Chat::getUserInput()
	{
		std::string str;
		cout << "Enter message (or q to quit) > ";
		getline(cin,str);
		return str;
	}

	/**
	 * This is where you should join the two threads spawned by
	 * your program: The sender thread and the receiver thread.
	 *
	 * Remember, you should be able to find thread IDs in member variables.
	 *
	 * Don't forget to check for errors.
	 *
	 * Do not join threads elsewhere in this program.
	 */
	void Chat::joinThreads()
	{
		//join threads
		int result;
		result=pthread_join(tid_sender_, NULL);
		if (result<0){
			cout << "join threads sender error!" << endl;
			throw 1;
		}
		result = pthread_join(tid_receiver_, NULL);
		if (result<0){
			cout << "join threads receive error!" << endl;
			throw 1;
		}
	}
}
