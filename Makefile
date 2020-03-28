
objs=main.o sqr.o util.o
srcdir=.

TOOLDIR=~/openwrt/staging_dir/toolchain-arm_cortex-a15+neon-vfpv4_gcc-8.4.0_musl_eabi/bin
CC=$(TOOLDIR)/arm-openwrt-linux-muslgnueabi-gcc
OBJDUMP=$(TOOLDIR)/arm-openwrt-linux-objdump
INCLUDES=-I$(srcdir) -I~/openwrt/staging_dir/target-arm_cortex-a15+neon-vfpv4_musl_eabi/usr/include -I~/openwrt/openwrt/staging_dir/toolchain-arm_cortex-a15+neon-vfpv4_gcc-8.4.0_musl_eabi/usr/include -I~/openwrt/staging_dir/toolchain-arm_cortex-a15+neon-vfpv4_gcc-8.4.0_musl_eabi/include/fortify -I~/openwrt/staging_dir/toolchain-arm_cortex-a15+neon-vfpv4_gcc-8.4.0_musl_eabi/include 
CFLAGS+=-I$(INCLUDES) -Os -pipe -fno-caller-saves -fno-plt -fhonour-copts -mfloat-abi=hard -fpic -fstack-protector -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro -ffunction-sections -fdata-sections -Wall
LDFLAGS=-L~/openwrt/staging_dir/target-arm_cortex-a15+neon-vfpv4_musl_eabi/usr/lib -L/h/ome/user/openwrt/staging_dir/target-arm_cortex-a15+neon-vfpv4_musl_eabi/lib -L~/openwrt/staging_dir/toolchain-arm_cortex-a15+neon-vfpv4_gcc-8.4.0_musl_eabi/usr/lib -L~/openwrt/staging_dir/toolchain-arm_cortex-a15+neon-vfpv4_gcc-8.4.0_musl_eabi/lib -fpic -specs=/home/user/openwrt/include/hardened-ld-pie.specs -znow -zrelro -Wl,--gc-sections 

TARGETS=ipq8065-sqrbug ipq8065-sqrbug.lst 

all: $(TARGETS)

# for simplicity assume all source depends on all headers
HEADERS=$(wildcard $(srcdir)/*.h *.h)
%.o : %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

ipq8065-sqrbug: $(HEADERS) $(objs) Makefile
	+$(CC) $(LDFLAGS) -o $@$(EXEEXT) $(objs)

ipq8065-sqrbug.lst: ipq8065-sqrbug
	$(OBJDUMP) -D $< > $@

clean:
	@rm -f $(objs)
	@rm -f $(TARGETS)
