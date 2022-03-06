CPPFLAGS = -D_XOPEN_SOURCE=700
CFLAGS = -std=c18 -Wpedantic -Wall -Wextra -Wconversion -Wwrite-strings -Werror -fstack-protector-all -fpie -ld -O2 -g 
#LDLIBS = -pthread -Wl,-z,relro,-z,now -fpie -lm 
LDFLAGS = 

objects = adresse_internet.o test.o
executable = test

all: $(executable)

$(executable): $(objects)
	$(CC) $(objects) $(LDFLAGS) -o $(executable)


adresse_internet.o: adresse_internet.c adresse_internet.h adresse_internet_type.h

test.o: test.c adresse_internet.h adresse_internet_type.h


clean:
	$(RM) -r *~ *.o $(executable) *.aux *.log *.synctex.gz 



