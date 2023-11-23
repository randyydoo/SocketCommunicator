#ifndef CPSC351_CHAT_HPP
#define CPSC351_CHAT_HPP


/**
 * You do not need to edit this file, although you can if you wish.
 * 
 * DO NOT modify existing prototypes. But you can add new helper functions.
 * 
 * If you decide to add new helper functions, place their prototypes here,
 * 	and definitions in Chat.cpp
 */


//
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


//
#include <string>


//
namespace CPSC351
{
	//
	class Chat
	{
		//
		public:
			
			/**
			 * Some class constants
			 * (your welcome)
			 */
			static const int INVALID_PORT = -1;
			static const int INVALID_SOCKET = -1;
			static const size_t BUFFER_SIZE = 65536;
			static constexpr const char * QUIT_SEQUENCE = "4f66dd5b7665259ee0cf676e987221f3\0";
			
			//
			Chat(int port_listen, std::string hostname, int port_out);
			~Chat();
			
			//
			void run();
			
			//
			bool isListeningSocketValid() const;
			void initListeningSocket();
			int getListeningSocket() const;
			void closeListeningSocket();
			
			//
			bool isOutboundSocketValid() const;
			void initOutboundSocket();
			int getOutboundSocket() const;
			void closeOutboundSocket();
			
			//
			void handleUserInput(std::string input);
			void sendChatMessage(std::string message);
			
			//
			std::string receiveChatMessage();
			void handleReceivedChatMessage(std::string message);
			
			///	Properly quit the chat session
			void quit();
			
			///	Return true if the chat app wants to quit; False otherwise
			bool isQuitting();
		
		//
		private:
			
			/**
			 * Flag that helps loops know when the program wants to quit
			 * true means the program wants to quit
			 * false means the program wants to keep running
			 */
			bool quitting_ = false;
			
			///	Port to listen on, for incoming data
			int port_listen_ = Chat::INVALID_PORT;
			
			///	Outgoing hostname and port to send chat data (your partner's address)
			std::string hostname_ = "";
			int port_out_ = Chat::INVALID_PORT;
			
			///	Listening (incoming data) socket
			int socket_listen_ = Chat::INVALID_SOCKET;
			
			///	Outgoing socket (sends data to your chat partner)
			int socket_out_ = Chat::INVALID_SOCKET;
			
			///	Thread ID for: The sender thread
			pthread_t tid_sender_;
			
			///	Thread ID for: The receiver thread
			pthread_t tid_receiver_;
			
			///	Return true if the socket identifier is valid, false otherwise
			bool isSocketValid(int sock) const;
			
			///	Properly close a socket
			void closeSocket(int& sock);
			
			///	Spawn both the sender and receiver threads
			void spawnThreads();
			
			///	Sender and receiver thread entry points
			void senderThread();
			void receiverThread();
			
			///	Get user input from the keyboard
			std::string getUserInput();
			
			///	Join the main thread with the sender and receiver threads
			void joinThreads();
			
			///	Friend functions!
			///	(your welcome)
			friend void* c_receiverThread(void* param);
			friend void* c_senderThread(void* param);
	};
}








#endif
