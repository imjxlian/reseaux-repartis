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
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return 1;
  }
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  int port = atoi(argv[1]);
  if (sock_fd < 0)
  {
    perror("Socket connection failed");
    return 1;
  }

  // bind socket to address
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_aton("192.168.60.174", &server_addr.sin_addr);

  int ret = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

  if (ret < 0)
  {
    perror("Bind failed");
    return 1;
  }

  // Listen for incoming connections
  ret = listen(sock_fd, 5);
  if (ret < 0)
  {
    perror("Listen failed");
    return 1;
  }

  // Create struct pollfd array
  int max_cons = 50;
  struct pollfd all_sock[max_cons];

  // Initialize struct pollfd array
  all_sock[0].fd = sock_fd;
  all_sock[0].events = POLLIN;
  all_sock[0].revents = 0;

  for (int i = 1; i < max_cons; i++)
  {
    all_sock[i].fd = -1;
    all_sock[i].events = 0;
    all_sock[i].revents = 0;
  }

  // Call poll function
  while (1)
  {
    poll(all_sock, max_cons, -1);

    // Check on listen socket and accept
    if (all_sock[0].revents & POLLIN)
    {
      struct sockaddr_in client_addr;
      socklen_t size_addr = sizeof(client_addr);
      int new_sockfd = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);

      if (new_sockfd < 0)
      {
        perror("Accept failed");
        return 1;
      }
      printf("New client (%s:%hu)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

      // Find an empty slot in all_sock and add the new socket
      for (int i = 1; i < max_cons; i++)
      {
        if (all_sock[i].fd == -1)
        {
          all_sock[i].fd = new_sockfd;
          all_sock[i].events = POLLIN;
          all_sock[i].revents = 0;
          break;
        }
      }
    }
    else
    {
      for (int i = 1; i < max_cons; i++)
      {
        // Check if there's an event on the current client socket
        if (all_sock[i].fd != -1 && all_sock[i].revents & POLLIN)
        {
          char buffer[1024];
          int nb_recu = recv(all_sock[i].fd, buffer, sizeof(buffer), 0);
          if (nb_recu == -1)
          {
            perror("Failed to retrieve messages");
            return 1;
          }
          if (nb_recu == 0)
          {
            // The client disconnected
            fprintf(stderr, "Connection terminated for client %d\n", i);
            close(all_sock[i].fd);
            all_sock[i].fd = -1; // Set the fd field to -1 to remove the client socket from the pollfd array
            all_sock[i].revents = 0;
          }
          else
          {
            buffer[nb_recu] = '\0';
            char message[1024];
            snprintf(message, sizeof(message), "[Client %d]: %s", i, buffer);

            fprintf(stdout, "%s", message);

            // Transfer message to all the others
            for (int j = 1; j < max_cons; j++)
            {
              if (all_sock[j].fd != -1 && i != j)
              {
                int n = write(all_sock[j].fd, message, strlen(message));
                if (n < 0)
                {
                  fprintf(stdout, "Error when sending the message to the client N°%d\n", j);
                  return 1;
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}
