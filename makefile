#Makefile for store
#Создание исполняемого модуля
binary: source.c
	gcc -o program source.c -lpthread
#End Makefile