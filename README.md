# FTP
This is a homework of the course Computer Network.

[Sockets API - wiki](https://en.wikipedia.org/wiki/Berkeley_sockets)

Instead of the API mentioned in the wiki, there are several another functions that may be useful:

- read / write
- Byte Ordering: htonl/htons/ntohl/ntohs <arpa/inet.h>
- htonl: host to net long(32 bits)
- htons: host to net short(16 bits)
- ntohl: net to host long(32 bits)
- ntohs: net to host short(16 bits) 

net is always big endian, but host is uncertain(mostly little endian)

Because of the big endian and little endian, byte ordering is staple.


[Ifilework](https://code.google.com/archive/p/ifilework/)

Ifilework is a project to develop a client and server ftp in C language using sockets api.

Just search "ftp server in c" in Google, you will find some helpful results.


[List of raw FTP commands](http://www.nsftools.com/tips/RawFTP.htm)

[How FTP actually works in practice](http://cr.yp.to/ftp.html)
- Look at the 1st session, which contains the **format of request and response**, such as the ending character.


Public FTP Server:
- ftp.gnu.org (anonymous only)
- ftp.acc.umu.se

[RFC 959](https://www.w3.org/Protocols/rfc959/)
- End-of-Line: \r\n


Fixed BUGs
- Blocked in the `while(1)` loop when read, because there is no '\n' from server.
- All the buffers such as cmd and state.message should be reset to 0 before next write.
- Use `htons` to set port, otherwise you get port 34074(0x851A) when you set port 6789(0x1A85).
- There is some difference between `inet_ntop` and `inet_ntoa` although they are similar, and the former is a new version of the latter.
- When you quit server before client, the code `close(sock_listen)` is not to be executed, which results in an error "Error bind(): Address already in use(98)" the next time you execute server. But it will return to normal in a few minutes.
- Define `char[BSIZE]` instead of `char *` inside of struct state.
- The size of verb in command must be over 5.
- The function accept() return a socket with a value of 0, because I close(sock_data) within cmd_quit(), and sock_data is 0.
In fact, stdin is 0 and stdout is 1, so I should not close the standard file descriptor.

Use orginal ftp tool to connect the server.
- open 127.0.0.1 6789