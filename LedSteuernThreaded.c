/* Einfacher Portscanner in C mit Threads
   Kompilieren mit:
   gcc LedSteuernThreaded.c -o LedSteuernThreaded -Wall -lpthread -lwiringPi
*/
//
//nc localhost 5000
//F9 Compilieren
//echo graz | nc localhost 5000

#include <stdio.h>
#include <stdlib.h> //standart libary
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <wiringPi.h>


#define BUFSIZE 1000
#define PORT 5000

#define pin8 6
#define pin4 5
#define pin2 4
#define pin1 1

// Warteschlange fuer anjommende Verbindungen
#define QUEUE 3

// Mirror-Funktion (Prototyp für pthread_create)

void *mirror(void* arg)
{
                int clientfd = *(int *)arg;  //typecast
                char inbuffer[BUFSIZE];
                
				// Message to client
				const char client_message[]="Programm: Leds ueber Netzwerk steuern; z.B: 15 -> alle Leds\n";
				write(clientfd, client_message,sizeof(client_message)); 
				
                //lesen der Zeichen aus dem Clientsocket -> inbuffer
                //count: Anzahl der gelesenen Bytes
                while(1)
                {
                int count = read(clientfd, inbuffer, sizeof(inbuffer));
				
				if(inbuffer[0] == 'q' || inbuffer[0] == 'Q') 
				{
					digitalWrite (pin8, LOW) ;
					digitalWrite (pin4, LOW) ;
					digitalWrite (pin2, LOW) ;
					digitalWrite (pin1, LOW) ;
					close(clientfd);
					return 0;
				}
													
				/* Control the LEDS */
                int leds = atoi(inbuffer);
                
                digitalWrite (pin8, (leds>>3)&1) ;
                digitalWrite (pin4, (leds>>2)&1) ;
                digitalWrite (pin2, (leds>>1)&1) ;
                digitalWrite (pin1, (leds>>0)&1) ;
                
				}
                close(clientfd);	// wird eigentlich nicht aufgerufen
                return NULL;

}

int main()  // argv = Argumente die ich über die Konsole übergebe
{
	// Pin Setup
	if(wiringPiSetup() == -1)
		return -1;
	pinMode (pin1, OUTPUT) ;
	pinMode (pin2, OUTPUT) ;
	pinMode (pin4, OUTPUT) ;
	pinMode (pin8, OUTPUT) ;
	
	
    //Socket
    int server_socket, rec_socket;
    unsigned int len;
    struct sockaddr_in serverinfo, clientinfo;


    //Serversocket konfigurieren
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // TCP
    serverinfo.sin_family = AF_INET; // IPv4
    //hoert auf allen Interfaces: 0.0.0.0 bzw. :::
    serverinfo.sin_addr.s_addr = htonl(INADDR_ANY);
    serverinfo.sin_port = htons(PORT);

    // Verbinde Socket mit IP-Adresse und Port
    if (bind(server_socket, (struct sockaddr *)&serverinfo,
		sizeof(serverinfo)) !=0){
                printf("Fehler Socket\n");
                return 1; //Rueckgabe

        }

        listen(server_socket, QUEUE);  //Server wartet auf connect vom Client
		
        //Endloschschleife zur Abarbeitung der Client-Anfragen
        while(1){

                printf("Server wartet...\n");
                //Verbindung vom Client eingetroffen
                rec_socket = accept(server_socket, (struct sockaddr *)&clientinfo, &len);
                printf("Verbindung von %s: %d\n", inet_ntoa(clientinfo.sin_addr),
						ntohs(clientinfo.sin_port));

                pthread_t child;  //Thread-Struktur
                //  Thread mit Funktion mirror(rec_socket) erzeugen
                if (pthread_create(&child, NULL, mirror, &rec_socket) != 0) {
                        perror("child error");  //Fehlerfall: Abbruch
                        return 1;
                }
                else {  // Kind erzeugt:
                        printf("Abgekoppelt!\n");
                        pthread_detach(child);  //abkoppeln vom Hauptprozessor

                }

        }


    return 0;
}
