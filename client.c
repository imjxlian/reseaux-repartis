#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

int main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "Usage %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0)
  {
    perror("Socket connection failed");
    return 1;
  }

  // Create the server addr struct
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_aton("192.168.60.174", &server_addr.sin_addr);

  int ret = connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (ret < 0)
  {
    perror("Connection failed");
    return 1;
  }

  struct pollfd fds[2];
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  fds[1].fd = sock_fd;
  fds[1].events = POLLIN;

  while (1)
  {
    int ret = poll(fds, 2, -1);
    if (ret < 0)
    {
      perror("Poll failed");
      return 1;
    }

    if (fds[0].revents & POLLIN)
    {
      char buffer[1024];
      fgets(buffer, 1023, stdin);

      // Check if the user input is "/quit"
      if (strcmp(buffer, "/quit\n") == 0)
      {
        break;
      }

      int n = write(sock_fd, buffer, strlen(buffer));
      if (n < 0)
      {
        perror("Error when sending the message");
        return 1;
      }
    }

    if (fds[1].revents & POLLIN)
    {
      char buffer[1024];
      int n = read(sock_fd, buffer, sizeof(buffer) - 1);
      if (n < 0)
      {
        perror("Error when reading the message");
        return 1;
      }
      if (n == 0)
      {
        fprintf(stderr, "Connection terminated by the server\n");
        break;
      }
      buffer[n] = '\0';
      printf("%s", buffer);
    }
  }

  // Close the socket
  shutdown(sock_fd, SHUT_RDWR);
  close(sock_fd);

  return 0;
}
