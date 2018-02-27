#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <p2pbackend.h>

/*create a node for p2p transfer*/
Node *node_create(uint16_t port, char* content, char* host) {
	Node n;
	n.port = port;
	n.content = content;
	n.host = host;
	return &n;
}

/*add a node to the list of nodes*/
void node_add(nlist* list, Node* node) {
	if(list->num_nodes >= list->max_nodes)
		error("Node list full!");
	list->nodes[list->num_nodes] = node;
	list->num_nodes++;
}

/*create a packet*/
Packet *packet_create(uint16_t source, uint16_t destination, uint16_t sequence, char* data) {
}

/*convert a packet struct to a string*/
char *packet_to_string(Packet p) {
}

/*convert a string to a packet*/
Packet *string_to_packet(char* buf) {
}