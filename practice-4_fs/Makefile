
#compile client.cpp -pthread and -lboost_system flag
#compile server.cpp along with ChatRoom.cpp and ChatSession.cpp -pthread and -lboost_system flag
all: server client
#compile crypto.cpp -pthread and -lboost_system flag to .o

#compile server.cpp along with ChatRoom.cpp and ChatSession.cpp -pthread and -lboost_system flag
server: server.cpp ChatRoom.cpp ChatSession.cpp crypto.cpp utils.cpp
	g++ -std=c++20 -o server server.cpp ChatRoom.cpp ChatSession.cpp crypto.cpp utils.cpp -pthread -lboost_system -lssl -lcrypto 
#compile client.cpp -pthread and -lboost_system flag to .o
client: client.cpp
	g++ -std=c++20 -o client client.cpp crypto.cpp utils.cpp -pthread -lboost_system -lssl -lcrypto
clean:
	rm -f server client