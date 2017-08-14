#include <arpa/inet.h>
#include <libgen.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

#include "client.h"
#include "ftree.h"
#include "server.h"

int rcopy_client(char *src, char *host, unsigned short port) {
	int sock_fd;
	if ((sock_fd = client_sock(host, port)) < 0) {
		fprintf(stderr,
				"error encountered during initializing client socket\n");
		return -1;
	}

	char *server_path = basename(src);

	if (traverse(sock_fd, src, server_path, host, port) < 0) {
		fprintf(stderr, "error encountered during traversing\n");
		return -1;
	}

	close(sock_fd);

	// wait

	return 0;
}

// FIXME: client pipe broken handle
void rcopy_server(unsigned short port) {
	int listen_fd;
	int nready, maxfd, client_fd;
	fd_set allset;
	fd_set rset;
	struct client *p;
	struct client *head = NULL;

	// Initialize server
	if ((listen_fd = server_sock()) < 0) {
		fprintf(stderr,
				"error encountered during initializing server socket\n");
		exit(-1);
	}

	// Initialize allset and add listen_fd to the
	// set of file descriptors passed into select
	FD_ZERO(&allset);
	FD_SET(listen_fd, &allset);
	// set maxfd to current listening fd
	maxfd = listen_fd;

	while (1) {
		// for (int i = 0; i < 20; i++, printf("%c", '-'))
		// 	;
		// printf("\n");
		rset = allset;

		struct sockaddr_in peer;
		peer.sin_family = PF_INET;
		unsigned int len;

		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if (nready < 0) {
			perror("rcopy_server: select");
			continue;
		}

		// printf("%d sockets are ready\n", nready);

		if (FD_ISSET(listen_fd, &rset)) {
			// server is ready to accept a new connection
			len = sizeof(peer);
			if ((client_fd =
					 accept(listen_fd, (struct sockaddr *)&peer, &len)) < 0) {
				perror("accept");
				continue;
			}
			FD_SET(client_fd, &allset);
			if (client_fd > maxfd) {
				maxfd = client_fd;
			}
			printf("connection from %s\n", inet_ntoa(peer.sin_addr));
			if (!(head = add_client(head, client_fd, peer.sin_addr))) {
				fprintf(stderr, "rcopy_server: add_client\n");
				continue;
			}
			printf("a new client %d connected\n", client_fd);
		}

		for (p = head; p != NULL; p = p->next) {
			if (FD_ISSET(p->fd, &rset)) {
				// printf("%d is ready\n", p->fd);
				int result = handle_client(p, head);
				if (result == HANDLE_DONE || result < 0) {
					if (result == HANDLE_DONE) {
						printf("HANDLE_DONE: handle for %d was successful\n",
							   p->fd);
					} else {
						fprintf(stderr, "rcopy_server: handle_client %d\n",
								p->fd);
					}
					FD_CLR(p->fd, &allset);
					if (close(p->fd) < 0) {
						perror("rcopy_server: close");
					}
					if (!(head = remove_client(head, p->fd))) {
						fprintf(stderr, "rcopy_server: remove_client\n");
					}
				}
			}
		}
	}
}
