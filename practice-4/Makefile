all: server client

server: server_main.cpp chat_server.cpp chat_session.cpp chat_room.cpp chat_message.cpp chat_client.cpp
	g++ -std=c++20 -o server server_main.cpp chat_server.cpp chat_session.cpp chat_room.cpp chat_message.cpp chat_client.cpp -lpthread -lboost_system -lboost_thread

client: client_main.cpp chat_client.cpp chat_message.cpp
	g++ -std=c++20 -o client client_main.cpp chat_client.cpp chat_message.cpp -lpthread -lboost_system -lboost_thread

clean:
	rm -f server client