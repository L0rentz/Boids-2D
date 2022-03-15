# CPP COMPILATION

CXXDIRECTORIES	=    src glad/include stb_image glm
CXXINCLOOP		=    $(foreach d, $(CXXDIRECTORIES), -I$d)

CXXFLAGS		=	-g3 -Wall -Wextra $(CXXINCLOOP) -O3

SRCCPP			=	main.cpp \
					stb_image/stb_image.cpp \
					src/Core.cpp \
					src/Boid.cpp \
					src/Utils.cpp

OBJCPP			=	$(SRCCPP:.cpp=.o)

# C COMPILATION

CDIRECTORIES	=    glad/include
CINCLOOP		=    $(foreach d, $(CDIRECTORIES), -I$d) -O3

CFLAGS			=	-g3 -Wall -Wextra $(CINCLOOP)

SRCC			= 	glad/src/glad.c

OBJC			=	$(SRCC:.c=.o)

# COMMON

NAME			=	Boids

LDFLAGS			=	-ldl -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lGL

RM				=	rm -rf

all: $(NAME)

$(NAME): $(OBJCPP) $(OBJC)
	g++ -o $(NAME) $(OBJCPP) $(OBJC) $(LDFLAGS)

# $(NAME): $(OBJCPP)
# 	g++ -o $(NAME) $(OBJCPP) $(LDFLAGS)

clean:
	$(RM) $(OBJCPP) $(OBJC)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re