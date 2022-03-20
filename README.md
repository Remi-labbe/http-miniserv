# HTTP-miniserv

Un serveur minimal developpe en C avec des sockets TCP.

## How to use

### Compile the project

```bash
make
```

### Start the Server

```bash
./server
```

The content to be served must be placed in the content folder.
This will be the folder the server is started in.

the server default on port 8080 to avoid needing sudo permissions to start it.
You can modify the port the server is listenning on in the config.h file.

### Possibilities

This basic server handles most basics filetypes. Only Http GET requests.
Can serve html, images, videos, javascript enabled.
