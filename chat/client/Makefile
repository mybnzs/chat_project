.PHONY:clean 
all:test_win test_client chat_client 

chat_client:chat_client.cc window.cc chat_client_main.cc 
	g++ $^ -o $@ -lpthread -ljsoncpp -lboost_system  -lboost_filesystem  -lncursesw  -std=c++11

test_win:window.cc 
	g++ $^ -o $@ -lpthread -lncursesw  -std=c++11 -lboost_system -lboost_filesystem -std=c++11 -DTEST_WINDOW 

test_client:chat_client.cc 
	g++ $^ -o $@ -lpthread -ljsoncpp -lboost_filesystem -lboost_system  -DTEST_CHAT_CLIENT

.PHONY:clean
clean:
	rm test_client  test_win chat_client 
