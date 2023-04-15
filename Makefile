ENV := gdb-debug
ENV := no-debug
ENV := valgrind-debug

STATIC_FLAGS := -static # enable static
STATIC_FLAGS := # disable static

VALGRIND_TOOL=helgrind
VALGRIND_TOOL=memcheck --leak-check=full --show-leak-kinds=all
VALGRIND_TOOL=memcheck --track-origins=yes --leak-check=full --show-leak-kinds=all --track-fds=yes -s
VALGRIND_TOOL=memcheck --track-origins=yes --leak-check=full --track-fds=yes -s

libimu_path=./lib/include
# libigen3_path=/usr/include/eigen3
# libtorch_path=../libtorch

libMHD_path=/usr/local/lib

GPP := g++
GCC := gcc

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

SERVER_LINKS := -lmicrohttpd -lgnutls
CLIENT_LINKS := -lcurl -lssl

ifeq ($(ENV),gdb-debug)
GPP += -Wall -g -O0 $(STATIC_FLAGS) 
GCC += -Wall -g -O0 $(STATIC_FLAGS) 
else
ifeq ($(ENV),valgrind-debug)
GPP += -Wall -g -O0 $(STATIC_FLAGS) 
GCC += -Wall -g -O0 $(STATIC_FLAGS) 
else
GPP += -Wall $(STATIC_FLAGS) 
GCC += -Wall $(STATIC_FLAGS) 
endif
endif

HEADERS := \
	-I $(libimu_path)/ 

m_clean:
	rm -f ./build/*.imux

m_server_basic_auth:
	$(GCC) $(HEADERS) \
	$(libimu_path)/../server_basic_auth.c $(SERVER_LINKS) -o ./build/server_basic_auth.imux

m_client_basic_auth:
	$(GCC) $(HEADERS) \
	$(libimu_path)/../client_basic_auth.c $(CLIENT_LINKS) -o ./build/client_basic_auth.imux

m_server_tls_auth:
	$(GCC) $(HEADERS) \
	$(libimu_path)/../server_tls_auth.c $(SERVER_LINKS) -o ./build/server_tls_auth.imux

m_client_tls_auth:
	$(GCC) $(HEADERS) \
	$(libimu_path)/../client_tls_auth.c $(CLIENT_LINKS) -o ./build/client_tls_auth.imux

server_basic_auth:
	make m_server_basic_auth
	echo "building [basic_auth]..."
ifeq ($(ENV),gdb-debug)
	gdb ./build/imu.imux
else
ifeq ($(ENV),valgrind-debug)
	valgrind --tool=$(VALGRIND_TOOL) ./build/server_basic_auth.imux
else
	./build/server_basic_auth.imux
endif
endif

client_basic_auth:
	make m_client_basic_auth
	echo "building [basic_auth]..."
ifeq ($(ENV),gdb-debug)
	gdb ./build/imu.imux
else
ifeq ($(ENV),valgrind-debug)
	valgrind --tool=$(VALGRIND_TOOL) ./build/client_basic_auth.imux
else
	./build/client_basic_auth.imux
endif
endif

server_tls_auth:
	make m_server_tls_auth
	echo "building [tls_auth]..."
ifeq ($(ENV),gdb-debug)
	gdb ./build/server_tls_auth.imux
else
ifeq ($(ENV),valgrind-debug)
	valgrind --tool=$(VALGRIND_TOOL) ./build/server_tls_auth.imux
else
	./build/server_tls_auth.imux
endif
endif

client_tls_auth:
	make m_client_tls_auth
	echo "building [tls_auth]..."
ifeq ($(ENV),gdb-debug)
	gdb ./build/client_tls_auth.imux
else
ifeq ($(ENV),valgrind-debug)
	valgrind --tool=$(VALGRIND_TOOL) ./build/client_tls_auth.imux
else
	./build/client_tls_auth.imux
endif
endif