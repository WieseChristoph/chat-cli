# Chat CLI

## Build

Run `make` command in root directory of this repo. This will create a executable named `chatcli`.

## Usage

### Start server

```
./chatcli serve <port>
```

### Connect to a server

```
./chatcli connect <ip-address> <port>
```

## Configuration

### Maximum number of clients and character limits

You can set the maximum number of clients and the character limits for the ip-address, username and message in `chat_server.h`.
