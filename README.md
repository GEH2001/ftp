# File Transfer Protocol

Simple implementation of ftp server and client.

## Server

### Supported Features
- Commands: USER PASS PASV LIST CWD PWD MKD RMD RETR STOR QUIT TYPE REST
- Multi-Clients based on multi-threading
- Non-blocking data transfer based on multi-threading

### Environment
Linux/GNU

### Usage
```bash
    $ cd server/
    $ make
    $ ./server -port [21] -root [/tmp]
```
`-port` specify the listening port

`-root` specify the initial working directory

### TODO
- Now I handle the `CWD` instruction through `chdir`, but when multiple users are connected, a separate current path should be maintained for each user in the structure `state`. That means server don't really change current working directory, but only stores the coming path with CWD to the user's state stucture.
- Fix `TYPE` function, because I transfer data only in binary type.

## Client Terminal

### Supported Features
- Commands: USER PASS PASV LIST CWD PWD MKD RMD RETR STOR QUIT TYPE REST

### Environment
- Python 3.12.0

### Usage
```bash
    $ cd client/
    $ python3 client.py
```

## Client GUI

### Supported Features
- Non-blocking GUI when transferring data
- You can pause and resume while transferring
- Show transfer progress with a progress bar

### Environment
- Windows
- Python 3.12.0
- PySide6 6.6.0

### Usage
```bash
    $ cd client/
    $ python3 client_gui.py
```
GUI is designed by `QT Designer`, saved as `mainwindow.ui`

Use `pyside6-uic` to generate python code

```bash
pyside6-uic mainwindow.ui -o ui_mainwindow.py
```
And include `ui_mainwindow.py` in `client_gui.py`

## Useful Tools

Public FTP Server
- ftp.gnu.org (anonymous only)
- ftp.acc.umu.se

FTP GUI Client
- FileZilla

## Reference

The most important reference
- [How FTP actually works in practice](http://cr.yp.to/ftp.html)
- [RFC 959](https://www.w3.org/Protocols/rfc959/)

Some useful repositories
- https://github.com/Siim/ftp
- https://github.com/DaHoC/ftpserver
- https://github.com/beckysag/ftp
- https://github.com/rovinbhandari/FTP

[Ifilework](https://code.google.com/archive/p/ifilework/) is a project to develop a client and server ftp in C language using sockets api.

In fact, `Ifilework` is the best project I have found, but I have no time reading it carefully.

Learn about Sockets API
- [Sockets API - wiki](https://en.wikipedia.org/wiki/Berkeley_sockets)
- [Socket Programming in C/C++](https://www.geeksforgeeks.org/socket-programming-cc/)
- [Sockets(The GNU C Library)](https://www.gnu.org/software/libc/manual/html_node/Sockets.html)

Several another functions that may be useful
- read / write / open / close
- Byte Ordering: htonl/htons/ntohl/ntohs <arpa/inet.h>

Something interesting
- [Socket Programming in C/C++: Handling multiple clients on server without multi-threading](https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/)