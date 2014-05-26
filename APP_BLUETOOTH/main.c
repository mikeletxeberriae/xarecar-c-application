/*
		CABECERA DEL FICHERO

*/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sco.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/l2cap.h>
#include <pthread.h>
#include "registrar_rfcomm.h"

/*************************
	CONSTANTES
**************************/
const int MAX_CONEXIONES=7; /*El bluetooth acepta unicamente 7 conexiones simultaneas, por ello el numero de sockets se limita a este valor*/
//const char * FINALIZE = "finalize" //comando finalizar


void * atender_clientes_bluetooth( void * arg );

/******************************************
* IMPLEMENTACION DE LAS FUNCIONES
*
*******************************************/

int main(int argc, char **argv)
{
    int port = 3;
    sdp_session_t* session = register_service(port);
    int dev_id;
    pthread_t lista_hilos[MAX_CONEXIONES] ;
    int i=0,j=0;
    int idSockets[MAX_CONEXIONES];
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    int s, client;
    socklen_t opt = sizeof(rem_addr);
    int r;

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    printf("socket() returned %d\n", s);

    // bind socket to port 1 of the first available
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) port;
    
    
    dev_id = hci_get_route(NULL); //Obtener la id del dispositivo bluetooth local
    printf("dev_id = %i\n",dev_id); 
    if (dev_id < 0) 
    { 
	printf("[!] Error. No hay ningun dispositivo Bluetooth local disponible.\n");  
	exit(1); 

    }
    r = bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
    printf("bind() on channel %d returned %d\n", port, r);

    // put socket into listening mode
    r = listen(s, 1);
    printf("listen() returned %d\n", r);

    while(1)
    {
    	// accept one connection
    	printf("calling accept()\n");
	client = accept(s, (struct sockaddr *)&rem_addr, &opt);
	printf("accept() returned %d\n", client);

	//convertir la direccion a string
	ba2str( &rem_addr.rc_bdaddr, buf );
	fprintf(stderr, "accepted connection from %s\n", buf);
	memset(buf, 0, sizeof(buf));
	
	//guardar el numero de socket, para luego poder pasar la direccion de ese socket
	idSockets[i]=client; 
	
	//Por cada socket crear un hilo, para encargarse de atender al cliente bluetooth
        pthread_create (&lista_hilos[i] , NULL , atender_clientes_bluetooth ,&idSockets[i]); 
	printf("Nº clientes conectados: %i\n",i+1);	
	i++;

    }

    /* El hilo "padre" (principal) espera a que los hilos "hijos" terminen */
    for(j=0;j<MAX_CONEXIONES;j++)
    {
    	pthread_join(lista_hilos[j],NULL);
    }

    /* close connection */
    close(s);

    /* Terminar el servicio */
    sdp_close( session );

    return 0;
}

/**
*@brief Esta es la funcion que ejecutará cada hilo
*@param arg. En este parametro se recogen los argumentos que se le hayan pasado a la funcion pthread_create()
*@return void*. Esta funcion puede devolver cualquier valor, el cual luego por ejemplo es posible utilizar para conocer si la funcion se ha ejecutado correctamente
*/
void * atender_clientes_bluetooth ( void * arg )
{
	int  *numSocket;
	int  bytes_read;
	char buf[1024] = { 0 };


	numSocket = (int *) arg;

	printf("Hilo atendendiendo socket %i\n",*numSocket);
	
	do
	{
		bytes_read = recv(*numSocket,buf,sizeof(buf),0);
		printf("Socket %i ha recibido: %s\n",*numSocket, buf);

	}while(strcmp(buf,"finalize")!=0); /* Comprobar que el cliente no llegue al final. (final indicado con "finalize")*/
	
	printf("El pasajero a llegado al destino.\n Enviando coste... ... ...\n");
	/*Enviar precio del viaje al cliente*/
	//bytes_read = send(*numSocket,buf,sizeof(buf),0);
	

	/* Cerrar el socket */
	close(*numSocket);
	return NULL ;
}

void arrancar_hilos()
{

	//Arrancar hilo cuenta kilometros


	//Arrancar contador de peajes



}

