#se define el programa que se quiere construir, el nombre del binario.
bin_PROGRAMS = app

#se especifica al linker que objetos externos añadir, en este caso la libreria de BLUEZ y la libreria de POSIX
app_LDADD = ${LIBBLUEZ} -lpthread

#se espefican los directorios de includes y de la libreria de BLUEZ. No se especifica el directorio de la libreria de POXIS, dado que se encuentra en el directorio estandar. Ademas se añade el flag "-static" para indicar que la compilacion se haga estaticamente, dentro del binario se ejecutara todo lo necesario sin tener que recurrir despues a elementos externos, de esta forma se evitarán posibles incompatibilidades.
AM_LDFLAGS = -static 
#-I/home/mikel/Desktop/Bluez_5/include/bluetooth -L/home/mikel/Desktop/Bluez_5/lib

#se especifican las fuentes con las que se construirá el binario espeficiado.
app_SOURCES = main.c registrar_rfcomm.c registrar_rfcomm.h
