#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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

  // Open and send the HTML file
  int fd = open("index.html", O_RDONLY);
  if (fd < 0)
  {
    perror("Open failed\n");
    close(new_sockfd);
    close(sock_fd);
    return 1;
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    perror("fstat");
    close(fd);
    close(new_sockfd);
    close(sock_fd);
    return 1;
  }

  char http_response[1024];
  sprintf(http_response, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %ld\r\n\r\n", (long)st.st_size);
  int sent = send(new_sockfd, http_response, strlen(http_response), 0);
  if (sent < 0)
  {
    perror("Failed to send response header");
    close(fd);
    close(new_sockfd);
    close(sock_fd);
    return 1;
  }

  // Send the file content byte by byte
  char ch;
  while (read(fd, &ch, 1) == 1)
  {
    if (send(new_sockfd, &ch, 1, 0) < 0)
    {
      perror("Failed to send file content");
      close(fd);
      close(new_sockfd);
      close(sock_fd);
      return 1;
    }
  }

  close(fd);
  close(new_sockfd);
  close(sock_fd);
  return 0;
}
