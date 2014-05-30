#se define el programa que se quiere construir, el nombre del binario.
bin_PROGRAMS = app

#se especifica al linker que objetos externos añadir, en este caso la libreria de BLUEZ y la libreria de POSIX
app_LDADD = ${BlueZ_LIBS} -lpthread

#se espefician los argumentos del compilador, como pueden ser los directorios de los includes
app_CFLAGS = ${BlueZ_CFLAGS}

#argumentos del linker
AM_LDFLAGS = ${SAVED_LDFLAGS}

#se especifican las fuentes con las que se construirá el binario espeficiado.
app_SOURCES = main.c registrar_rfcomm.c registrar_rfcomm.h
