## Usage

```
make module
sudo rmmod target/rw_iter.ko
sudo insmod target/rw_iter.ko

make target/main
# Sync mode
sudo ./target/main
# Async (io_uring) mode
sudo ./target/main 1

# Verify with kernel messages
sudo dmesg
```

## Troubleshooting

### liburing

If liburing is out of sync, one may need to manually compile it.
Assuming the checkout in the sibling directory:

```
cd ..
git clone https://github.com/axboe/liburing.git
cd -
make -C ../liburing
make target/main_upstream_liburing
sudo ./target/main_upstream_liburing
```

### Valgrind

On Archlinux, you may use Valgrind as:

```
sudo DEBUGINFOD_URLS="https://debuginfod.archlinux.org" valgrind ./target/main_upstream_liburing
```

## References

* http://krishnamohanlinux.blogspot.com/2015/02/getuserpages-example.html
* https://lwn.net/Articles/625077/
* https://github.com/cirosantilli/linux-kernel-module-cheat
* https://elinux.org/images/b/b0/Introduction_to_Memory_Management_in_Linux.pdf
* https://github.com/torvalds/linux/blob/master/Documentation/DMA-API-HOWTO.txt
* https://elinux.org/images/3/32/Pinchart--mastering_the_dma_and_iommu_apis.pdf
* http://www.mouritzen.dk/aix-doc/en_US/a_doc_lib/libs/ktechrf1/pinu.htm#A275X932d2
* http://www.mouritzen.dk/aix-doc/en_US/a_doc_lib/aixprggd/kernextc/dma_transfers.htm
* https://lwn.net/Articles/234617/
* https://static.lwn.net/kerneldoc/driver-api/dmaengine/provider.html
* https://lwn.net/Articles/753027/
* https://linuxplumbersconf.org/event/2/contributions/126/attachments/136/168/LPC_2018_gup_dma.pdf
* https://lkml.org/lkml/2014/12/4/704
