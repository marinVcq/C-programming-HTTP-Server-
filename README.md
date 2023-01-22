# Serveur HTTP from scratch: Tout ce que vous devez savoir

Courte Introduction

## Le modèle OSI

Modele de Communication en réseau, l'OSI est une norme qui standardise la communication
entre 2 machines dans un réseau.  [Voir Modele OSI: wikipedia](https://fr.wikipedia.org/wiki/Mod%C3%A8le_OSI). La couche Transport regroupe l'ensemble des protocoles utilisés dans la transmission de donnée, ici nous allons nous interesser au modèle TCP/IP il existe d'autre protocole comme par exemple UDP qui contrairement à TCP/IP ne garantie pas une transmission fiable des données.

### TCP/IP

Via TCP/IP une connexion s'établie en 3 temps [Three-way-handshake](https://fr.wikipedia.org/wiki/Three-way_handshake)
Pour implementer ce protocole en C/C++ voici une idée globale de la structure d'un segment TCP.

[TCP Segment](./webroot/TCPSegment.png)

### Socket programming

La programmation socket permet la communication en réseau [Socket programming: Wikipedia](https://fr.wikipedia.org/wiki/Berkeley_sockets), disponible pour UNIX et Windows ici nous allons nous interesser a l'API Winsock2 de Windows qui 
va nous permettre de structuré nos segments et d'établir une  connexion entre un client et le serveur via la 
procédure suivante:

1. Initialisation de Winsock
2. Création du Socket
3. Liaison du Socket
4. Ecoute du réseau
5. Transmission de donnée
6. Destruction du socket

#### Création du Socket

1. Déclaration  d'un Objet `WSADATA` qui contiendra les informations windows relative au socket
l'objet est initialisé par la fonction `WSAStartup` [doc](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup)

2. La fonction `socket()` permet la création du socket serveur, elle prend en parametre le type d'adresse, 
le protocole et le type de socket. La fonction renvoie un objet `SOCKET` [doc](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-socket)

Dans notre Cas les valeurs initiales suivantes:

```
int iFamily = AF_UNSPEC;
int iType = SOCK_STREAM;
int iProtocol = IPPROTO_TCP;
```

#### Liaison du Socket (Bind)

1. Assignation d'un port et d'une adresse à notre socket. Pour cela on stock ces parametres dans un object `SOCKADDR_IN`
et on initialise la famille d'adresse (AF_INET pour Ipv4), le Port d'écoute et enfin l'adresse IP (INADDR_ANY pour utiliser
n'importe quelle adresse disponible en local) [doc](https://learn.microsoft.com/fr-fr/windows/win32/api/winsock/ns-winsock-sockaddr_in)

```
   ServerAddr.sin_family = AF_INET;
   ServerAddr.sin_port = htons(Port);    
   ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
```

2. La fonction `Bind()` associe le socket au parametre de l'Object `SOCKADDR_IN` [doc](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-bind)

#### Ecoute et attente de connexion (Listen)

La fonction `listen` [doc](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-listen) place le socket en état d'attente de connexion sur son port. le parametre `backlog` permet de
definir le nombre max de réponse placé en liste d'attente pour être accepté ou refusé par la fonction `accept` [doc](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept). Par défault les sockets fonctionne de maniere asynchrone ou bloquant.

Le parametre `SOCKET socket` de la fonction `accept` répresente l'Objet dans la file d'attente de connexion via la précédente fonction `listen`

#### Transmission des données

Une fois la conexion acceptée le client et le serveur peuvent mutuellment se transmettre des données avec la fonction `send` [doc](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send) et `recv` [doc](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv) Les données à transmettre ou recevoir sont stockées dans un buffer.

#### Fermeture du socket

Aprés avoir effectué l'opération souhaité on ferme le flux par l'appel de la fonction `closesocket` [doc](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-closesocket).


## Le Protocol HTTP

Maintenant que notre serveur TCP/IP est set up il est nécéssaire que les données envoyées vers le navigateur depuis le serveur respectent le protocole de communication client-serveur sur le web. Pour cela il sera necessaire:

1. Initialisé la connexion par une requête
2. Gérer les différentes requête envoyé par le navigateur

### Le modèle client-serveur

- Le client (navigateur) demande une page web
- le serveur, qui héberge les ressources traite les requêtes et sert les clients

#### Modèle de communication

La réponse HTTP du serveur doit correspondre au standard HTTP pour cela il va falloir écrire l'En-tête et l'en,voyé aprés chaque requête HTTP du client.

[image header http](./webroot/response-headers.jpg)























