ENV := gdb-debug
ENV := valgrind-debug
ENV := no-debug

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
# $(libMHD_path)/libmicrohttpd.a 

# GPP += -Wno-pointer-arith

# sdl_cflags := $(shell pkg-config --cflags sdl2 SDL2_mixer SDL2_image SDL2_ttf)
# # # # # sdl_libs := $(shell pkg-config --libs sdl2 SDL2_mixer SDL2_image SDL2_ttf)
# GPP += $(sdl_cflags)
# GPP += $(sdl_libs)

# torch_cflags := -D_GLIBCXX_USE_CXX11_ABI=0 -std=gnu++17
# torch_libs := -std=c++17 -L${libtorch_path}/lib -Wl,-R${libtorch_path}/lib -ltorch -ltorch_cpu -lc10
# GPP += $(torch_cflags)
# GPP += $(torch_libs)

HEADERS := \
	-I $(libimu_path)/ \
	# -I $(libMHD_path)/ \
	# -I ${libtorch_path}/lib \
	# -I ${libtorch_path}/include/ \
	# -I ${libtorch_path}/include/torch/csrc/api/include

# -I $(libigen3_path)/ \
# -I $(libimu_path)/ \

m_clean:
	rm -f ./build/*.imux

m_server_basic_auth:
	$(GCC) $(HEADERS) \
	$(libimu_path)/../server_basic_auth.c $(SERVER_LINKS) -o ./build/server_basic_auth.imux

m_client_basic_auth:
	$(GPP) $(HEADERS) \
	$(libimu_path)/../client_basic_auth.cpp $(CLIENT_LINKS) -o ./build/client_basic_auth.imux

m_server_tls_auth:
	$(GCC) $(HEADERS) \
	$(libimu_path)/../server_tls_auth.c $(SERVER_LINKS) -o ./build/server_tls_auth.imux

m_client_tls_auth:
	$(GPP) $(HEADERS) \
	$(libimu_path)/../client_tls_auth.cpp $(CLIENT_LINKS) -o ./build/client_tls_auth.imux

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