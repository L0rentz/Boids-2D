NAME			=	Boids

CXX				=	g++

DIRECTORIES		=    ./src
INCLOOP			=    $(foreach d, $(DIRECTORIES), -I$d)

CXXFLAGS		=	-g3 -Wall -Wextra $(INCLOOP)

LDFLAGS			=	-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lGL

SRC				=	main.cpp \
					src/Core.cpp \
					src/Boid.cpp

OBJ				=	$(SRC:.cpp=.o)

RM				=	rm -rf

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re