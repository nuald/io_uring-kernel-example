CONFIG_MODULE_SIG=n

obj-m += rw_iter.o

.PHONY: module
module: target/rw_iter.ko

target/main: main.c | target
	gcc -Wall -Wextra -pedantic -o "$@" main.c -luring

target/main_upstream_liburing: main.c | target
	gcc -I../liburing/src/include/ -L../liburing/src/ -Wall -Wextra -o "$@" main.c -luring

target/rw_iter.ko: rw_iter.c | target
	make -C /lib/modules/$(shell uname -r)/build MO=$(PWD)/target M=$(PWD) src=$(PWD) modules

target:
	mkdir -p "$@"

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
