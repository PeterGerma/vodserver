#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct Node_t {
	uint16_t port; //backend port number
	char* content; //path to content
	char* host; //p2p host ip
} Node;

typedef struct Packet_t {
	uint16_t source; //source port
	uint16_t destination; //destination port
	uint32_t sequence; //sequence number
	uint32_t ack; //acknowledgement number
	uint16_t window; //window size
	uint16_t checksum; //checksum
	char* buf; //data buffer
} Packet;

typedef struct Node_List {
	uint32_t num_nodes; //number of nodes in the list
	uint32_t max_nodes; //maximum nodes
	Node *(*nodes); //list of nodes
} nlist;

/*create a node for p2p transfer*/
Node *node_create(uint16_t port, char* content, char* host);

/*create a node list*/
nlist* nlist_create(uint32_t max_nodes);

/*add a node to the list of nodes*/
void node_add(nlist list, Node node);

/*looks for node with desired content path*/
Node* find_content(char* content, nlist* nl);

/*create a packet*/
Packet *packet_create(uint16_t source, uint16_t destination, uint16_t sequence, char* data);

/*convert a packet struct to a string*/
char *packet_to_string(Packet* p);

/*convert a string to a packet*/
Packet *string_to_packet(char* buf);