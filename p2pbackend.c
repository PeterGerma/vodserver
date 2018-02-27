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
Node* node_create(uint16_t port, char* content, char* host) {
	Node *n;
	n->port = port;
	n->content = content;
	n->host = host;
	return n;
}

nlist* nlist_create(uint32_t max_nodes) {
	nlist* nl;
	nl->max_nodes = max_nodes;
	nl->nodes[max_nodes];
	nl->num_nodes = 0;
	return nl;
}

/*add a node to the list of nodes*/
void node_add(nlist* list, Node* node) {
	if(list->num_nodes >= list->max_nodes)
		error("Node list full!");
	list->nodes[list->num_nodes] = node;
	list->num_nodes++;
}

/*looks for node with desired content path*/
Node* find_content(char* content, nlist* nl) {
	int i;
	for(i=0; i<nl->num_nodes; i++) {
		if(strcmp(content, nl->nodes[i]->content) == 0) {
			return nl->nodes[i];
		}
	}
	return NULL;
}

/*create a packet*/
Packet *packet_create(uint16_t source, uint16_t destination, uint16_t sequence, char* data) {
	Packet* p;
	p->source = source;
	p->destination = destination;
	p->sequence = sequence;
	p->buf = data;
	return p;
}

/*convert a packet struct to a string*/
char *packet_to_string(Packet* p) {
	char* buf = "";
	char* numbuf[11];

	sprintf(numbuf, p->source);
	strcat(buf, numbuf);

	strcat(buf, ",");

	sprintf(numbuf, p->destination);
	strcat(buf, numbuf);

	strcat(buf, ",");

	sprintf(numbuf, p->sequence);
	strcat(buf, numbuf);

	strcat(buf, ",");

	sprintf(numbuf, p->ack);
	strcat(buf, numbuf);

	strcat(buf, ",");

	sprintf(numbuf, p->window);
	strcat(buf, numbuf);

	strcat(buf, ",");

	sprintf(numbuf, p->checksum);
	strcat(buf, numbuf);

	strcat(buf, ",");

	strcat(buf, p->buf);
}

/*convert a string to a packet*/
Packet *string_to_packet(char* buf) {
	Packet* p;
	p->source = (uint16_t)atoi(strtok(buf, ","));
	p->destination = (uint16_t)atoi(strtok(NULL, ","));
	p->sequence = (uint32_t)atoi(strtok(NULL, ","));
	p->ack = (uint32_t)atoi(strtok(NULL, ","));
	p->window = (uint16_t)atoi(strtok(NULL, ","));
	p->checksum = (uint16_t)atoi(strtok(NULL, ","));
	p->buf = strtok(NULL, ",");
}