# Eduardo Stefanel Paludo - GRR20210581
# Natael Pontarolo Gomes - GRR20211786

flags = -Wall
name = multi-partition

all: $(name)

$(name): main.o chrono.o verifica_particoes.o
	mpicc -o $(name) main.o chrono.o verifica_particoes.o $(flags)

main.o: main.c
	mpicc -c main.c $(flags)

chrono.o: chrono.c
	mpicc -c chrono.c $(flags)

verifica_particoes.o: verifica_particoes.c
	mpicc -c verifica_particoes.c $(flags)

clean:
	rm -f *~ *.o

purge: clean
	rm -f $(name)
