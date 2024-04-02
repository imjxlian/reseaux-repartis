#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return 1;
  }

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  int port = atoi(argv[1]);
  if (sock_fd < 0)
  {
    perror("Socket creation failed");
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("Bind failed");
    close(sock_fd);
    return 1;
  }

  if (listen(sock_fd, 5) < 0)
  {
    perror("Listen failed");
    close(sock_fd);
    return 1;
  }

  struct sockaddr_in client_addr;
  socklen_t size_addr = sizeof(client_addr);
  int new_sockfd = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);
  if (new_sockfd < 0)
  {
    perror("Accept failed");
    close(sock_fd);
    return 1;
  }

  printf("New HTTP client connected: %s:%hu\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

  char buffer[4096];
  int nb_recu = recv(new_sockfd, buffer, sizeof(buffer) - 1, 0);
  if (nb_recu < 0)
  {
    perror("Failed to receive data");
    close(new_sockfd);
    close(sock_fd);
    return 1;
  }

  buffer[nb_recu] = '\0';
  printf("Received HTTP request:\n%s\n", buffer);

  char body[] = "<html><head><title>Not a test</title></head><body style=\"background-color: cyan;\"><h1>Hello, World!</h1></body></html>";
  char http_response[1024];

  sprintf(http_response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %lu\r\n\r\n%s", strlen(body), body);
  int sent = send(new_sockfd, http_response, strlen(http_response), 0);
  if (sent < 0)
  {
    perror("Failed to send response");
    close(new_sockfd);
    close(sock_fd);
    return 1;
  }

  close(new_sockfd);
  close(sock_fd);
  return 0;
}
