/**
 * DO NOT MODIFY THIS SOURCE FILE.
 */


/**
 * Notes to self
 * 
 * Send to 127.0.0.1 as opposed to localhost; localhost seems to have caused some sort of strange error related to broadcast addresses.
 * I may or may not have my hosts file setup correctly, and I don't care. Just use 127.0.0.1
 */

//
#include <chrono>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <tuple>
#include <vector>


//
#include "Chat.hpp"
#include "puhp-tests/PuhPTests.hpp"


//
using std::cout, std::cin, std::endl;
using std::ifstream, std::ofstream;
using std::string, std::to_string;
using std::stringstream;
using std::vector;


//
using
	PuhPTests::Net,
	PuhPTests::OutputCapture,
	PuhPTests::Process,
	PuhPTests::Random,
	PuhPTests::Test,
	PuhPTests::Tests
	;


//
using CPSC351::Chat;


//	Globals
const string QUIT_SEQUENCE="4f66dd5b7665259ee0cf676e987221f3\0";
const string HOSTNAME = "127.0.0.1";
const int PORT1 = 8822;
const string PORT1_s = to_string(PORT1);
const int PORT2 = 8823;
const string PORT2_s = to_string(PORT2);


//	PROTO
bool isValidSocket(int fd);
void sleep(int ms);
void testInvalid(Tests& tests);
void testSocketManip(Tests& tests);
void testTransmission(Tests& tests);
void testHugeTransmission(Tests& tests);


//	Race conditions abound; Don't be like me
class Chat_RAII
{
	//
	public:
		
		//
		Chat_RAII(int port_listen = PORT1, string hostname = HOSTNAME, int port_out = PORT2)
		{
			//
			this->chat_ = new Chat(port_listen, hostname, port_out);
			
			//
			this->chat_->initListeningSocket();
			this->chat_->initOutboundSocket();
		}
		
		~Chat_RAII()
		{
			//
			this->joinThreads();
			
			//
			if ( this->chat_ != nullptr ) {
				delete this->chat_;
				this->chat_ = nullptr;
			}
		}
		
		//
		void send(string s, bool join = false)
		{
			//
			if ( this->tid_send_ != 0 ) {
				pthread_cancel(this->tid_send_);
			}
			this->next_send_ = s;
			//cout << "send() ==> " << this->next_send_ << endl;
			
			//
			auto result = pthread_create(&this->tid_send_, NULL, &Chat_RAII::send_entry, this);
			if ( result != 0 ) {
				throw std::runtime_error(string("Failed to create RAII send thread: ") + strerror(errno));
			}
			
			//	Try to give the caller more time for the thread to engage
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			
			//
			if ( join == true ) {
				this->joinThread(this->tid_send_);
			}
		}
		
		//
		void receive(bool join = false)
		{
			//
			if ( this->tid_receive_ != 0 ) {
				pthread_cancel(this->tid_receive_);
			}
			
			//
			auto result = pthread_create(&this->tid_receive_, NULL, &Chat_RAII::receive_entry, this);
			if ( result != 0 ) {
				throw std::runtime_error(string("Failed to create RAII receive thread: ") + strerror(errno));
			}
			
			//	Try to give the caller more time for the thread to engage
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			
			//
			if ( join == true ) {
				this->joinThread(this->tid_receive_);
			}
		}
		
		void joinThreads()
		{
			this->joinThread(this->tid_receive_);
		}
		
		void joinThread(pthread_t& tid)
		{
			//
			if ( tid == 0 ) {
				return;
			}
			
			//
			//cout << "Trying to join thread: " << tid << endl;
			
			//
			auto result = pthread_join(tid, NULL);
			if ( result != 0 ) {
				throw std::runtime_error(string("Failed to join RAII thread: ") + strerror(errno));
			}
			tid = 0;
		}
		
		void killThreads()
		{
			this->killThread(this->tid_receive_);
		}
		
		void killThread(pthread_t& tid)
		{
			//
			auto result = pthread_cancel(tid);
			if ( result != 0 ) {
				throw std::runtime_error(string("Failed to cancel RAII thread: ") + strerror(errno));
			}
			tid = 0;
		}
		
		//
		static void* send_entry(void * param)
		{
			//
			Chat_RAII* p = static_cast<Chat_RAII*>(param);
			
			//
			auto s = p->next_send_;
			
			//
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			//cout << "Send entry! ==> " << s << endl;
			p->chat()->sendChatMessage(s);
			
			return nullptr;
		}
		
		//
		static void* receive_entry(void * param)
		{
			Chat_RAII* p = static_cast<Chat_RAII*>(param);
			
			//
			string s = p->chat()->receiveChatMessage();
			p->lastReceived(s);
			
			//cout << "RECEIVED: " << s << endl;
			
			return nullptr;
		}
		
		Chat* chat()
		{
			return this->chat_;
		}
		
		string lastReceived()
		{
			return this->last_received_;
		}
		void lastReceived(string s)
		{
			this->last_received_ = s;
		}
	
	//
	private:
		Chat* chat_ = nullptr;
		pthread_t tid_receive_= 0;
		pthread_t tid_send_= 0;
		string next_send_ = "";
		string last_received_ = "";
};


//
int main()
{
	//
	cout << "Begin tests" << endl;
	
	//
	Tests tests(20);
	
	//
	vector<std::function<void(Tests&)>> functions = {
		testInvalid,
		testSocketManip,
		testTransmission,
		testHugeTransmission,
	};
	
	//
	for ( auto f : functions ) {
		f(tests);
	}
	
	//
	tests.run(true);
	
	//
	cout << "Tests complete" << endl;
	
	return 0;
}

bool isValidSocket(int fd)
{
	//
	int val;
	unsigned int val_length = sizeof(int);
	
	//	An invalid socket will return -1 for this call
	int result = getsockopt(
		fd,
		SOL_SOCKET,		//	Socket level
		SO_ACCEPTCONN,	//	Random option
		&val, &val_length
	);
	if ( result == 0 ) {
		return true;
	}
	
	return false;
}

//
void sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

//
void testInvalid(Tests& tests)
{
	//
	auto t = tests.createTest("Invalid Stuff");
	
	//
	t->setFixedPointsPossible(2);
	t->setNormalizedPointsPossibleTarget(5);
	
	//	Throw when trying to sendChatMessage with an invalid socket
	t->assertException(
		[]()
		{
			//
			Chat chat(PORT1, HOSTNAME, PORT2);
			chat.sendChatMessage("Oh dear");
			
		}, 1, "Trying to send with an invalid socket should throw an exception"
	);
	
	//	Throw when trying to receiveChatMessage with an invalid socket
	t->assertException(
		[]()
		{
			//
			Chat chat(PORT1, HOSTNAME, PORT2);
			chat.receiveChatMessage();
			
		}, 1, "Trying to receive with an invalid socket should throw an exception"
	);
}

//
void testSocketManip(Tests& tests)
{
	//
	auto t = tests.createTest("Socket Manip");
	
	//
	t->setFixedPointsPossible(11);
	t->setNormalizedPointsPossibleTarget(10);
	
	//
	t->assertNoException(
		[t]()
		{
			//
			Chat chat(PORT1, HOSTNAME, PORT2);
			int sock_listen, sock_out;
			
			//	Sockets should start as invalid
			sock_listen = chat.getListeningSocket();
			sock_out = chat.getOutboundSocket();
			t->assertFalse(isValidSocket(sock_listen), 1, "Listening socket should start in an invalid state.");
			t->assertFalse(isValidSocket(sock_out), 1, "Listening socket should start in an invalid state.");
			
			//	Chat should understand validity, at a basic level
			t->assertEqual(chat.isListeningSocketValid(), isValidSocket(sock_listen), 1, "Chat should understand whether the listening socket is valid (at a superficial level, at least)");
			t->assertEqual(chat.isOutboundSocketValid(), isValidSocket(sock_out), 1, "Chat should understand whether the outbound socket is valid (at a superficial level, at least)");
			
			//	Init and see if they become valid
			chat.initListeningSocket();
			sock_listen = chat.getListeningSocket();
			t->assertTrue(isValidSocket(sock_listen), 1, "Listening socket should be in a valid state after initialization");
			//
			chat.initOutboundSocket();
			sock_out = chat.getOutboundSocket();
			t->assertTrue(isValidSocket(sock_out), 1, "Outbound socket should be in a valid state after initialization");
			
			//	Again, chat should understand validity, at a basic level
			t->assertEqual(chat.isListeningSocketValid(), isValidSocket(sock_listen), 1, "Chat should understand whether the listening socket is valid (at a superficial level, at least)");
			t->assertEqual(chat.isOutboundSocketValid(), isValidSocket(sock_out), 1, "Chat should understand whether the outbound socket is valid (at a superficial level, at least)");
			
			//	Close and see if they become invalid (use the old values)
			chat.closeListeningSocket();
			t->assertFalse(isValidSocket(sock_listen), 1, "Listening socket should be invalid after closing.");
			t->assertTrue(isValidSocket(sock_out), 0, "Outbound socket should still be valid at this point.");
			//
			chat.closeOutboundSocket();
			t->assertFalse(isValidSocket(sock_out), 1, "Outbound socket should be invalid after closing.");
			
		}, 1, "Socket Manip tests"
	);
}

//
void testTransmission(Tests& tests)
{
	//
	auto t = tests.createTest("Transmission");
	
	//
	t->setFixedPointsPossible(30);
	t->setNormalizedPointsPossibleTarget(10);
	
	//
	t->assertNoException(
		[]()
		{
			Chat_RAII chat;
		}, 1, "No exception creating Chat and initializing sockets."
	);
	
	//	Check basic send and receive
	t->assertNoException(
		[t]()
		{
			//
			Net net;
			Random rand;
			
			Chat_RAII chat;
			
			int r;
			
			//	Attempt to send the chat program some data
			chat.receive();
			r = rand.get(1000000, 2000000);
			net.sendData(string("Sending to Chat app: ") + to_string(r) + '\0', HOSTNAME, PORT1, false);
			sleep(100);
			string lastReceived = chat.lastReceived();
			t->assertNotEqual(lastReceived.find(to_string(r)), string::npos, 10, "Check that the chat app can receive a message.");
			t->assertEqual(lastReceived.find(to_string(r + 1)), string::npos, 1, "Check that the chat app isn't being weird about received messages.");
			chat.joinThreads();
			
			//	Attempt to receive some data from the chat program
			r = rand.get(1000000, 2000000);
			chat.send(string("Sending from chat app: ") + to_string(r) + '\0', false);
			string data = net.receiveData(PORT2, 2, false);
			t->assertTrue(data.find(to_string(r)) != string::npos, 10, "Check that the chat app can send a message.");
			t->assertFalse(data.find(to_string(r) + "1") != string::npos, 1, "Check that the chat app isn't being weird about sending messages.");
			chat.joinThreads();
			
		}, 1, "Checking basic send and receive"
	);
	
	//	Chat app should actually print received messages with cout
	t->assertNoException(
		[t]()
		{
			//
			Chat_RAII chat;
			Chat* ch = chat.chat();
			Net net;
			Random rand;
			string stdout;
			
			//
			int r = rand.get(1000000000, 2000000000);
			
			//	Send it our random string
			OutputCapture cap;
			ch->handleReceivedChatMessage(string("Hey --- ") + to_string(r) + " ---");
			cap.endCapture();
			stdout = cap.getStdout();
			t->assertFalse(stdout.find(to_string(r)) == string::npos, 1, "Chat app should actually print received messages, through handleReceivedChatMessage()");
		}, 1, "Checking for printed received messages, via cout"
	);
	
	//	Check that the chat responds to the quit sequence
	t->assertNoException(
		[t]()
		{
			//
			Chat_RAII
				chat1(PORT1, HOSTNAME, PORT2),
				chat2(PORT2, HOSTNAME, PORT1)
				;
			Chat* ch1 = chat1.chat();
			Chat* ch2 = chat2.chat();
			Net net;
			Random rand;
			
			//
			int r = rand.get(1000000000, 2000000000);
			
			//	Chat apps should not initially be quitting
			t->assertFalse(ch1->isQuitting(), 1, "Chat app 1 should not initially be quitting");
			t->assertFalse(ch2->isQuitting(), 0, "Chat app 2 should not initially be quitting");
			
			//	Send Chat1 the quit string
			ch1->handleReceivedChatMessage(QUIT_SEQUENCE);
			
			//	Send Chat2 the quit string, buried inside other characters
			ch2->handleReceivedChatMessage(to_string(r) + QUIT_SEQUENCE + to_string(r));
			
			//	They should now both be quitting
			t->assertTrue(ch1->isQuitting(), 1, "Chat app 1 should be quitting after being sent the quit string by itself");
			t->assertTrue(ch2->isQuitting(), 1, "Chat app 2 should be quitting after being sent the quit string, embedded between other characters");
			
		}, 1, "Checking that the chat app responds to the quit sequence"
	);
}

//
void testHugeTransmission(Tests& tests)
{
	//
	auto t = tests.createTest("Huge Transmission");
	
	//
	t->setFixedPointsPossible(7);
	t->setNormalizedPointsPossibleTarget(10);
	
	//	Some last points for successfully receiving a huge message
	t->assertNoException(
		[t]()
		{
			//
			Net net;
			Random rand;
			Chat_RAII chat;
			int r;
			
			//	Build a huge message
			r = rand.get(1000000000, 2000000000);
			string message = to_string(r);
			for ( int i = 0; i < 12; i++ ) {
				message += message;
			}
			
			//	Attempt to send the huge message to the chat program
			chat.receive();
			net.sendData(message + '\0', HOSTNAME, PORT1, false);
			chat.joinThreads();
			string lastReceived = chat.lastReceived();
			t->assertTrue(lastReceived.find(message) != string::npos, 5, "Check that the chat app can receive a huge message (probably by looping and concatenating calls to recv).");
			t->assertTrue(lastReceived.find(message + "a") == string::npos, 1, "Check that the chat app can receive a huge message (probably by looping and concatenating calls to recv), exactly.");
			
		}, 1, "Checking for success receiving a huge message"
	);
}












