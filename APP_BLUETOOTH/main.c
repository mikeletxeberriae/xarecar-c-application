/*
		CABECERA DEL FICHERO

*/

#include <stdio.h>
#include <stdlib.h>
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
#include <semaphore.h>


/*************************
 * 	ESTRUCTURAS
 * ***********************/
typedef struct datos_conexion
{
 int num_socket_conexion;
 int nConexion; 
}DATOS_CONEXION;

/*************************
	CONSTANTES
**************************/
#define MAX_CONEXIONES 7 /*El bluetooth acepta unicamente 7 conexiones simultaneas, por ello el numero de sockets se limita a este valor*/

const char * COMANDO_LLEGADA = "finalize"; 
const char * COMANDO_START_LECTURA = "start";
const int TIEMPO_PASO_KM = 10; //=segundos
const float PRECIO_COMBUSTIBLE = 0.05; //euros

/**************************
 * VARIABLES GLOBALES
***************************/

int numConexiones=0;
int numKilometros=0;
int start=0;
int nueva_conexion=0;
int salida_conexion=0;
float costePorUsuarios[MAX_CONEXIONES]; //con const int MAX_CONEXIONES falla por ello se ha puesto #define MAX_CONEXIONES

//semaforos
sem_t sem_conexiones;
sem_t sem_kilometros;
sem_t sem_start;
sem_t sem_nueva_conexion;
sem_t sem_salida_conexion;
sem_t sem_costePorUsuarios;
sem_t sem_esperarCoste;

/*******************************
 * DEFINICION DE LAS FUNCIONES
 * 
 *******************************/

void * atender_clientes_bluetooth( void * arg );
void * lecturaDatosCoche(void * arg);
void * calculadorCoste(void * arg);


/******************************************
* IMPLEMENTACION DE LAS FUNCIONES
*
*******************************************/

/**
*@brief Esta es la funcion principal de la aplicacion. Se encarga de publicar el servicion de bluetooth, aceptar conexiones externas y asignar a cada conexion un hilo y de iniciar el hilo encargado de leer el kilometraje del coche.
*@param argc. Numero de argumentos que ha recibido la funcion main
*@param **argv. La lista de argumentos que ha recibido la funcion main
*@return int. Devuelve un 0 si se ha ejecutado con normalidad.
*/
int main(int argc, char **argv)
{
    int port = 3,
	dev_id,
	i=0,
	j=0,
	s,
	client,
	r,
	idSockets[MAX_CONEXIONES];
    
    sdp_session_t* session;
    pthread_t lista_hilos[MAX_CONEXIONES],
              hilo_lector_datos,
              hilo_calculador;
    struct sockaddr_rc loc_addr = { 0 }, 
		        rem_addr = { 0 };
    char buf[1024] = { 0 };
    socklen_t opt = sizeof(rem_addr);
    DATOS_CONEXION datosConexion[MAX_CONEXIONES];
    
    //Inicializar semaforos
    sem_init(&sem_conexiones,0,1);
    sem_init(&sem_kilometros,0,1);
    sem_init(&sem_start,0,1);
    sem_init(&sem_nueva_conexion,0,1);
    sem_init(&sem_salida_conexion,0,1);
    sem_init(&sem_costePorUsuarios,0,1);
    sem_init(&sem_esperarCoste,0,0);
    
    //Crear hilo lector de datos
    pthread_create (&hilo_lector_datos , NULL , lecturaDatosCoche ,NULL); 
    
    //Crear hilo calculador
    pthread_create (&hilo_calculador , NULL , calculadorCoste ,NULL);
    
    //Iniciar el servicio
    session = register_service(port);

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    printf("socket() returned %d\n", s);

    // bind socket to port 1 of the first available
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) port;
    
    //Comprobacion del adaptador local de bluetooth
    dev_id = hci_get_route(NULL); //Con el parametro NULL devuelve la id del dispositivo bluetooth local
    printf("dev_id = %i\n",dev_id); 
    if (dev_id < 0) 
    { 
	printf("[!] Error. No hay ningun dispositivo Bluetooth local disponible.\n");  
	exit(1); 

    }
    
    //Bind
    r = bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
    printf("bind() on channel %d returned %d\n", port, r);

    //put socket into listening mode
    r = listen(s, 1);
    printf("listen() returned %d\n", r);

    while(1)
    {
	sem_wait(&sem_conexiones);
	
	if(numConexiones < MAX_CONEXIONES) //se controlan el numero de conexiones.
	{
	    sem_post(&sem_conexiones);
	    
	    // accept one connection
	    printf("calling accept()\n");
	    client = accept(s, (struct sockaddr *)&rem_addr, &opt);
	    printf("accept() returned %d\n", client);

	    //convertir la direccion a string
	    ba2str( &rem_addr.rc_bdaddr, buf );
	    fprintf(stderr, "accepted connection from %s\n", buf);
	    memset(buf, 0, sizeof(buf));
	    
	    
	    sem_wait(&sem_conexiones);
	    numConexiones++; //Incrementar la conexion
	    //guardar el numero de socket, para luego poder pasar la direccion de ese socket
	    idSockets[numConexiones-1]=client; 
	    
	    //semaforo_costeUsuarios
	    sem_wait(&sem_costePorUsuarios);
	    costePorUsuarios[numConexiones-1]=0;
	    sem_post(&sem_costePorUsuarios);
	    
	    datosConexion[numConexiones-1].num_socket_conexion= client;
	    datosConexion[numConexiones-1].nConexion = numConexiones;
	    
	    //Por cada socket crear un hilo, para encargarse de atender al cliente bluetooth
	    pthread_create (&lista_hilos[numConexiones-1] , NULL , atender_clientes_bluetooth ,&datosConexion[numConexiones-1]); 
	    
	    printf("Nº clientes conectados: %i\n",numConexiones);
	    
	}
	sem_post(&sem_conexiones);

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
*@brief Esta es la funcion que ejecutará cada hilo, encargado de atender a una de las conexiones del cliente. Se encarga de recoger el kilometro inicial y la final para que le calcule el coste del viaje al cliante una vez llegado al destino (indicando con el comando "finalize"). Si el cliente es el conductor este le puede enviar un start para iniciar la lectura.
*@param arg. En este parametro se recogen los argumentos que se le hayan pasado a la funcion pthread_create()
*@return void*. Esta funcion puede devolver cualquier valor, el cual luego por ejemplo es posible utilizar para conocer si la funcion se ha ejecutado correctamente
*/
void * atender_clientes_bluetooth ( void * arg )
{
    DATOS_CONEXION  *datosConexion;
    int  bytes_read;
    char buffer_recepcion[1024] = { 0 };
    char buffer_envio[128] = { 0 };
    float coste=0;
    
    datosConexion = (DATOS_CONEXION *) arg;

    printf("Hilo atendendiendo socket %i\n",datosConexion->num_socket_conexion);

    sem_wait(&sem_nueva_conexion);
    nueva_conexion=1;
    sem_post(&sem_nueva_conexion);
    do
    {
	bytes_read = recv(datosConexion->num_socket_conexion,buffer_recepcion,sizeof(buffer_recepcion),0);
	printf("Socket %i ha recibido: %s\n",datosConexion->num_socket_conexion, buffer_recepcion);


	if(strcmp(buffer_recepcion,COMANDO_START_LECTURA)==0)
	{
	    printf("¡Se ha recibido start!\n");
	    sem_wait(&sem_start);
	    start=1;
	    sem_post(&sem_start);
	}
	
    }while(strcmp(buffer_recepcion,COMANDO_LLEGADA)!=0); /* Comprobar que el cliente no llegue al final. (final indicado con "finalize")*/

    sem_wait(&sem_salida_conexion);
    salida_conexion=1;
    sem_post(&sem_salida_conexion);
    
    printf("Esperando coste... ... ...\n");

    sem_wait(&sem_esperarCoste);
    
    sem_wait(&sem_costePorUsuarios);
    coste = costePorUsuarios[datosConexion->nConexion-1];
    sem_post(&sem_costePorUsuarios);
    
    /*Enviar precio del viaje al cliente*/
    sprintf(buffer_envio,"%.2f",coste);
    bytes_read = send(datosConexion->num_socket_conexion,buffer_envio,sizeof(buffer_envio),0);
    printf("El pasajero a llegado al destino.\n Enviando coste... ... ...\n");
    printf("Coste: %s €\n",buffer_envio);
    
    sem_wait(&sem_conexiones);
    numConexiones--; //se decrementa el numero de conexiones
    sem_post(&sem_conexiones);
    
    /* Cerrar el socket */
    close(datosConexion->num_socket_conexion);
    return NULL ;
}

/**
*@brief Esta es la funcion que ejecutará el hilo encardado de realizar la lectura. Se simula la lectura con el incremento de un contador que indica el numero de kilometros realizados.
*@param arg. En este parametro se recogen los argumentos que se le hayan pasado a la funcion pthread_create(), a este hilo no se le ha asignado ningun parametro.
*@return void*. Esta funcion puede devolver cualquier valor, el cual luego por ejemplo es posible utilizar para conocer si la funcion se ha ejecutado correctamente
*/
void * lecturaDatosCoche(void * arg)
{
  
  printf("Hilo lector en marcha, esperando comando 'start'...\n");
  while(1)
  {
      sem_wait(&sem_start);
      if(start==1)
      {
	sem_post(&sem_start);
	
	sem_wait(&sem_kilometros);
	printf("Leyendo kilometro actual: %i\n", numKilometros);
	numKilometros++;
	sem_post(&sem_kilometros);
	
	sleep(TIEMPO_PASO_KM);
	
      }
      else
      {
	sem_post(&sem_start);
      }
  }
      return NULL;
}
/**
*@brief Esta es la funcion que ejecutará el hilo encardado de calcular el coste de cada pasajero. Controla la conexion y desconexion de los clientes. En cada uno de los instantes actualiza el coste de cada pasajeros.
*@param arg. En este parametro se recogen los argumentos que se le hayan pasado a la funcion pthread_create(), a este hilo no se le ha asignado ningun parametro.
*@return void*. Esta funcion puede devolver cualquier valor, el cual luego por ejemplo es posible utilizar para conocer si la funcion se ha ejecutado correctamente
*/
void * calculadorCoste(void * arg)
{
  float costeIndividual=0;
  int i=0;
  
  
  printf("Hilo calculador en marcha...\n");
  
  while(1)
  {
    //Comprobar si se ha conectado algun pasajero
    sem_wait(&sem_nueva_conexion);
    if(nueva_conexion==1)
    {
      nueva_conexion=0;
      
      sem_post(&sem_nueva_conexion);
 
      sem_wait(&sem_conexiones);

      if(numConexiones-1>0) //Si es la primera conexion no se calcula el coste. Si antes de esta nueva conexion habia mas conexiones, se calcula su coste
      {
	sem_wait(&sem_kilometros);
	costeIndividual = (float)((numKilometros*PRECIO_COMBUSTIBLE)/(numConexiones-1));
	printf("Calculado el coste... ... ...\n");
	sem_post(&sem_kilometros);
      
	sem_wait(&sem_costePorUsuarios);
	
	for(i=0;i<numConexiones-1;i++) //-1 porque no se tiene en cuenta la nueva conexion
	{
	  
	  costePorUsuarios[i] = costePorUsuarios[i] + costeIndividual;
	  
	}
	sem_post(&sem_costePorUsuarios);
	
	printf("Coste individual: %.2f\n",costeIndividual);
      }
      
      sem_post(&sem_conexiones);
      
      sem_wait(&sem_kilometros);
      numKilometros=0;
      sem_post(&sem_kilometros);
      

    }
    else
    {
      sem_post(&sem_nueva_conexion);
    }
    
    //Comprobar si se ha desconectado algun pasajero
    sem_wait(&sem_salida_conexion);
    if(salida_conexion==1)
    {

      salida_conexion=0;
      
      sem_post(&sem_salida_conexion);
      
      sem_wait(&sem_kilometros);

      sem_wait(&sem_conexiones);
      printf("Conexiones : %i\n",numConexiones);
      costeIndividual = (float)((numKilometros*PRECIO_COMBUSTIBLE)/(numConexiones));
      
      printf("Calculado el coste... ... ...\n");

      numKilometros=0;
      sem_post(&sem_kilometros);
  
      printf("Coste individual: %.2f\n",costeIndividual);
      
      sem_wait(&sem_costePorUsuarios);
      for(i=0;i<numConexiones;i++)
      {
	
	costePorUsuarios[i] = costePorUsuarios[i] + costeIndividual;
	
      }

      sem_post(&sem_costePorUsuarios);
      
      //Avisar de que se ha actualizado el coste
      sem_post(&sem_esperarCoste);
      sem_post(&sem_conexiones);
      
    }
    else
    {
      sem_post(&sem_salida_conexion);
    }
    
  }
  
  return NULL;
}