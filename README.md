# Waajacu tls_server
This is a starting bench for projects that use Client and Server side tls authentication

# Educational Resources
https://curl.se/docs/manual.html

https://www.gnu.org/software/libmicrohttpd/tutorial.html

https://github.com/cnlohr/rawdrawandroid (how to build the client files as an android app build in c)
# Requires 
- libmicrohttpd
# Build instructions
download latest version of libcurl https://curl.se/download.html 
```
[user@waajacu ~]$ cd ./path/to/Downloads
[user@waajacu Downloads]$ .tar zxvf curl.*.tar.gz
[user@waajacu Downloads]$ cd ./curl.*
```
download latest version of libmicrohttpd https://www.gnu.org/software/libmicrohttpd/
```
[user@waajacu ~]$ cd ./path/to/Downloads
[user@waajacu Downloads]$ .tar zxvf libmicrohttpd.*.tar.gz
[user@waajacu Downloads]$ cd ./libmicrohttpd.*
```
Install libmicrohttpd
```
[user@waajacu libmicrohttpd]$ ./configure --with-gcrypt=/usr/lib64/ --with-gnutls=/usr/lib64/
[user@waajacu libmicrohttpd]$ make
[user@waajacu libmicrohttpd]$ sudo make install
```
Install dynamic dependencies
```
[user@waajacu ~]$ glibc++
[user@waajacu ~]$ libstdc++
```
Install static dependencies (optional, required to compile static)
```
[user@waajacu ~]$ gnutls-static
[user@waajacu ~]$ glibc++-static
[user@waajacu ~]$ libstdc++-static
```
# Server Authentication instructions
Generate the server side identity files
```
[user@waajacu ~]$ cd /path/to/proyect/tls_server
[user@waajacu tls_server]$ cd ./access
[user@waajacu access]$ openssl genrsa 2048 > server.key.pem
[user@waajacu access]$ openssl req -new -x509 -nodes -sha1 -days 365 -key server.key.pem > server.cert
[user@waajacu access]$ cat server.cert server.key.pem > server.pem
```
# Client Authehticantion steps
Generate the client side identity files
```
[user@waajacu access]$ openssl genrsa -out client.key.pem 2048
[user@waajacu access]$ openssl req -x509 -sha256 -new -nodes -key client.key.pem -days 3650 -out client.pem
```
# Run Instructions
## Basic authentication server
Run a server with basic authentication privilege access
```
[user@waajacu ~]$ cd /path/to/proyect/tls_server
[user@waajacu tls_server]$ cd /path/to/proyect/tls_server
[user@waajacu tls_server]$ make server_basic_auth
[user@waajacu tls_server]$ ./build/server_basic_auth.imux
```
Here enter localhost:8888 in a web browser to check the server, or build the basic_auth_client executable 
```
[user@waajacu ~]$ cd /path/to/proyect/tls_server
[user@waajacu tls_server]$ make client_basic_auth
[user@waajacu tls_server]$ ./build/client_basic_auth.imux
```
## TLS authentication server
Run a server with tls authentication privilege access
```
[user@waajacu ~]$ cd /path/to/proyect/tls_server
[user@waajacu tls_server]$ cd /path/to/proyect/tls_server
[user@waajacu tls_server]$ make server_tls_auth
[user@waajacu tls_server]$ ./build/server_tls_auth.imux
```
To access the TLS server, the tls_auth_client executable is required:  
```
[user@waajacu ~]$ cd /path/to/proyect/tls_server
[user@waajacu tls_server]$ make client_tls_auth
[user@waajacu tls_server]$ ./build/client_tls_auth.imux
```