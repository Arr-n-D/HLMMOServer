FROM library/alpine:3.20.0 as gamenetworkingsockets

RUN apk add git=2.45.1-r0 \
            cmake=3.29.3-r0 \
            ninja-is-really-ninja=1.12.1-r0 \
            gcc=13.2.1_git20240309-r0 \
            musl-dev=1.2.5-r0 \
            g++=13.2.1_git20240309-r0 \
            openssl-dev=3.3.0-r2 \
            protobuf-dev=24.4-r1

WORKDIR /app

RUN git clone https://github.com/ValveSoftware/GameNetworkingSockets.git /app
RUN git reset --hard 8f4d800b2ac9690bb049d8012135dfedd19488b9
RUN git submodule update --init --recursive -j 1

WORKDIR /app/build

RUN cmake -G Ninja ..
RUN ninja

FROM library/alpine:3.20.0 as server

WORKDIR /app

RUN apk add gcc=13.2.1_git20240309-r0 \
            g++=13.2.1_git20240309-r0 \
            libcrypto3=3.3.0-r2 \
            xmake=2.9.1-r0 \
            git=2.45.1-r0 \
            7zip=23.01-r0 \
            curl=8.7.1-r0 \
            linux-headers=6.6-r0 \
            libprotobuf=24.4-r1

COPY ./xmake.lua /app

RUN xmake --root --yes require

COPY --from=gamenetworkingsockets /app/build/bin/libGameNetworkingSockets.so /usr/lib/
COPY ./main.cpp /app
COPY ./src/ /app/src/
COPY ./include/ /app/include/

RUN xmake --root --diagnosis --yes

FROM library/alpine:3.20.0

RUN apk add libprotobuf=24.4-r1

COPY --from=gamenetworkingsockets /app/build/bin/libGameNetworkingSockets.so /usr/lib/

WORKDIR /app

COPY --from=server /app/build/linux/x86_64/release/HLMMOServer /app/

CMD [ "/app/HLMMOServer" ]
