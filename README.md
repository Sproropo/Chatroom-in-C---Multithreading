<<<<<<< HEAD
# Chatroom-in-C---Multithreading
SimpleChat is a command-line chatroom application written in C using multithreading. No encryption or authentication implemented
=======
SimpleChat is a command-line chatroom application written in C using multithreading. No encryption or authentication implemented

server.c: Contains the main server-side logic.
client.c: The client-side code that handles user input.

# Setup & Usage

## Build Instructions:
gcc -o server server.c -lpthread && gcc -o client client.c -lpthread

## Run the app
Start the server: ./server <port_number> 
Connect clients: ./client <server_ip> <username> <port_number> 
>>>>>>> dcf5cd4 (Completed)
