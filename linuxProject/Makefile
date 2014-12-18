NAME=client
CFLAGS=-o $(NAME)
GTKFLAGS=`pkg-config --cflags --libs gtk+-2.0 gmodule-export-2.0`
SRCS = client.c
CC=gcc
 
# top-level rule to create the program.
all : main
 
# compiling the source file.
main : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(GTKFLAGS)
 
# cleaning everything that can be automatically recreated with "make".
clean :
	rm -f $(NAME)

