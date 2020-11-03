#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

void error(const char *msg) { perror(msg); exit(0); }

int main()
{
    /* first what are we going to send and where are we going to send it? */
    int portno = 8888;
    char *host = "localhost";

    struct hostent *server;
    struct sockaddr_in sock_addr;
    socklen_t addrlen;
    int sock_listen, sock_conn, bytes, sent, received, total;
    char message[1024],response[4096];

    /* create the socket */
    sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_listen < 0)
      error("ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) error("ERROR, no such host");

    /* fill in the structure */
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(portno);
    memcpy(&sock_addr.sin_addr.s_addr, server->h_addr_list[0],
           server->h_length);

    /* connect the socket */
    if (bind(sock_listen, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0)
      error("ERROR binding");

    if (listen(sock_listen, 10) < 0) {
      error("ERROR listening");
    }

    if((sock_conn = accept(sock_listen,(struct sockaddr *) &sock_addr, &addrlen)) < 0)
    {
        error("ERROR accepting");
    }

    if (sock_conn > 0)
    {
        printf("The Client is connected...\n");
    }

    recv(sock_conn,message,sizeof(message),0);

    printf("Request:\n%s", message);
    write(sock_conn, "HTTP/1.1 200 OK\n",16);
    write(sock_conn, "Content-length: 46\n",19);
    write(sock_conn, "Content-Type: text/html\n\n",25);
    write(sock_conn, "<html><body><H1>Hello world</H1></body></html>",46);
    close(sock_conn);

    close(sock_listen);

    return 0;
    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
      bytes = write(sock_listen, message + sent, total - sent);
      if (bytes < 0)
        error("ERROR writing message to socket");
      if (bytes == 0)
        break;
      sent += bytes;
        } while (sent < total);

    /* receive the response */
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
      bytes = read(sock_listen, response + received, total - received);
      if (bytes < 0)
        error("ERROR reading response from socket");
      if (bytes == 0)
        break;
      received += bytes;
        } while (received < total);

    if (received == total)
        error("ERROR storing complete response from socket");

    /* close the socket */
    close(sock_listen);

    /* process response */
    printf("Response:\n%s\n",response);

    return 0;
}
