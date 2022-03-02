# Encrypted-Messaging (server-and-clients)

C program for Asymetric encryption, one server and many clients.


for server

gcc server.c -o s
./s 8888

(or some port number)

for client 

gcc client.c -o c -lcrypto

client 1

./c 127.0.0.1 8888 private1.pem public2.pem

client 2

./c 127.0.0.1 8888 private2.pem public1.pem

(the same port number used for server)


after that enter text on any terminal

Refer to the screenshots for examples
