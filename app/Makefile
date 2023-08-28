CFLAGS+=-pthread -I /home/ldy/nfs_rootfs/cjson/cJSON -I /home/ldy/100ask_imx6ull-sdk/Linux-4.9.88/curl-8.2.1/out/include -I /home/ldy//nfs_rootfs/zlib/include
LDFLAGS+=-pthread -L /home/ldy/nfs_rootfs/cjson/lib -L /home/ldy/100ask_imx6ull-sdk/Linux-4.9.88/curl-8.2.1/out/lib -L /home/ldy//nfs_rootfs/zlib/lib
LIBS+=-lcurl -lcjson -lz
CC = arm-buildroot-linux-gnueabihf-gcc

all:weather_test temperature_test clean irda_test pir_test ifa

weather_test:weather_test.o weather.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

temperature_test:temperature_test.o temperature.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

irda_test:irda_test.o irda.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

pir_test:pir_test.o pir.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

ifa:ifa.o pir.o irda.o temperature.o weather.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)


clean:
	rm -rf *.o