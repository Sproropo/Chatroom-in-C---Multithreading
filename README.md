# Chatroom-in-C---Multithreading
This is a command-line chatroom application written in C using multithreading. No encryption or authentication implemented.
This chatroom application was developed from scratch. (Note: This is an older project originally created and distributed by me in 2021 before being uploaded in 2024 to GitHub.)

- server.c: Contains the main server-side logic.
- client.c: The client-side code that handles user input.

# Setup & Usage

## Build Instructions:
gcc -o server server.c -lpthread && gcc -o client client.c -lpthread

## Run the app
- Start the server: ./server <port_number> 
- Connect clients: ./client <server_ip> <username> <port_number>

## Extras
- This project was developed from scratch. (Note: This is an older project originally created and distributed by me in 2021 before being uploaded in 2024 to GitHub.)
