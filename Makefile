NAME = webserv

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = 	srcs/main.cpp \
		srcs/Config/Config.cpp \
		srcs/Server/Socket.cpp \
		srcs/Server/Server.cpp \
		srcs/Server/Client.cpp \
		srcs/Request/Request.cpp \
		srcs/Request/RequestBody.cpp \
		srcs/Response/Response.cpp \
		srcs/Response/ResponseUtils.cpp \
		srcs/Config/ServerConfig.cpp \
		srcs/Config/LocationConfig.cpp \
		srcs/Config/ConfigParser.cpp \
		srcs/Cgi/CgiExecutor.cpp \
		
OBJS = $(SRCS:.cpp=.o)

INCLUDES = -I include

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

parsing:
	chmod +x check_parsing.sh
	./check_parsing.sh

.PHONY: all clean fclean re parsing