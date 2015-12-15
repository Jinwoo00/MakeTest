export arch_arm_v7_dir=$(CURDIR)/arch/arm/v7
export lib_c_src_ARCH_ARM_dir=$(CURDIR)/lib/c/src/arch-arm
export core_dir=$(CURDIR)/core
export core_libhw_dir=$(CURDIR)/core/libhw
export core_vdev_dir=$(CURDIR)/core/vdev
export tests_dir=$(CURDIR)/tests
export tests_libs_dir=$(CURDIR)/test/libs
export arch_arm_dir=$(CURDIR)/arch/arm
export drivers_dir=$(CURDIR)/drivers
export lib_c_src_dir=$(CURDIR)/lib/c/src
export lib_c_src_SYS_BAREMETAL_ARCH_ARM_dir=$(CURDIR)/lib/c/src/sys-baremetal/arch-arm
export lib_c_src_SYS_BAREMETAL_dir=$(CURDIR)/lib/c/src/sys-baremetal

arch-arm-v7-source-mks=$(shell if [ -d $(arch_arm_v7_dir) ]; then find $(arch_arm_v7_dir) -iname "sources.mk" | sort -r; fi)
lib-c-src-ARCH_ARM-source-mks=$(shell if [ -d $(lib_c_src_ARCH_ARM_dir) ]; then find $(lib_c_src_ARCH_ARM_dir) -iname "sources.mk" | sort -r; fi)
core-source-mks=$(shell if [ -d $(core_dir) ]; then find $(core_dir) -iname "sources.mk" | sort -r; fi)
core-libhw-source-mks=$(shell if [ -d $(core_libhw_dir) ]; then find $(core_libhw_dir) -iname "sources.mk" | sort -r; fi)
core-vdev-source-mks=$(shell if [ -d $(core_vdev_dir) ]; then find $(core_vdev_dir) -iname "sources.mk" | sort -r; fi)
tests-source-mks=$(shell if [ -d $(tests_dir) ]; then find $(tests_dir) -iname "sources.mk" | sort -r; fi)
tests-libs-source-mks=$(shell if [ -d $(tests_libs_dir) ]; then find $(tests_libs_dir) -iname "sources.mk" | sort -r; fi)
arch-arm-source-mks=$(shell if [ -d $(arch_arm_dir) ]; then find $(arch_arm_dir) -iname "sources.mk" | sort -r; fi)
drivers-source-mks=$(shell if [ -d $(drivers_dir) ]; then find $(drivers_dir) -iname "sources.mk" | sort -r; fi)
lib-c-src-source-mks=$(shell if [ -d $(lib_c_src_dir) ]; then find $(lib_c_src_dir) -iname "sources.mk" | sort -r; fi)
lib-c-src-SYS_BAREMETAL-ARCH_ARM-source-mks=$(shell if [ -d $(lib_c_src_SYS_BAREMETAL_ARCH_ARM_dir) ]; then find $(lib_c_src_SYS_BAREMETAL_ARCH_ARM_dir) -iname "sources.mk" | sort -r; fi)
lib-c-src-SYS_BAREMETAL-source-mks=$(shell if [ -d $(lib_c_src_SYS_BAREMETAL_dir) ]; then find $(lib_c_src_SYS_BAREMETAL_dir) -iname "sources.mk" | sort -r; fi)

include $(arch-arm-v7-source-mks)
include $(lib-c-src-ARCH_ARM-source-mks)
include $(core-source-mks)
include $(core-libhw-source-mks)
include $(core-vdev-source-mks)
include $(tests-source-mks)
include $(tests-libs-source-mks)
include $(arch-arm-source-mks)
include $(drivers-source-mks)
include $(lib-c-src-source-mks)
include $(lib-c-src-SYS_BAREMETAL-ARCH_ARM-source-mks)
include $(lib-c-src-SYS_BAREMETAL-source-mks)
include config.mk

arch-arm-v7-y=$(foreach src, $(arch-arm-v7-src-y), $(src))
lib-c-src-ARCH_ARM-y=$(foreach src, $(lib-c-src-ARCH_ARM-src-y), $(src))
core-y=$(foreach src, $(core-src-y), $(src))
core-libhw-y=$(foreach src, $(core-libhw-src-y), $(src))
core-vdev-y=$(foreach src, $(core-vdev-src-y), $(src))
tests-y=$(foreach src, $(tests-src-y), $(src))
tests-libs-y=$(foreach src, $(tests-libs-src-y), $(src))
arch-arm-y=$(foreach src, $(arch-arm-src-y), $(src))
drivers-y=$(foreach src, $(drivers-src-y), $(src))
lib-c-src-y=$(foreach src, $(lib-c-src-src-y), $(src))
lib-c-src-SYS_BAREMETAL-ARCH_ARM-y=$(foreach src, $(lib-c-src-SYS_BAREMETAL-ARCH_ARM-src-y), $(src))
lib-c-src-SYS_BAREMETAL-y=$(foreach src, $(lib-c-src-SYS_BAREMETAL-src-y), $(src))


OBJS 		:= $(arch-arm-v7-y:.S=.o) $(lib-c-src-ARCH_ARM-y:.S=.o) $(lib-c-src-ARCH_ARM-y:.c=.o) \
			   $(core-y:.c=.o) $(core-libhw-y:.c=.o) $(core-vdev-y:.c=.o)\
			   $(tests-y:.c=.o) $(tests-libs-y:.c=.o)\
			   $(arch-arm-y:.c=.o) $(drivers-y:.c=.o)\
			   $(lib-c-src-y:.c=.o) $(lib-c-src-SYS_BAREMETAL-ARCH_ARM-y:.c=.o)\
			   $(lib-c-src-SYS_BAREMETAL-y:.c=.o)

BIN			= $(BUILD_DIR)/$(TARGET).bin
LD_SCRIPT	= $(PROJECT).lds.S
OUTPUT 		= $(TARGET).axf
MAP			= $(PROJECT).map

CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
NM			= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy


INCLUDES	= -I $(INC) -I $(INC)/$(LIB)/c -I $(INC)/$(LIB)/bsd -I $(INC)/$(LIB)
INCLUDES	+= -I $(INC)/$(DRV)
INCLUDES    += -I $(CORE)/libhw -I tests -I $(CORE) -I $(CORE)/include -I include/lib -I ./lib -I ./include/core
INCLUDES	+= -I tests/libs

CPPFLAGS	= $(CONFIG_FLAG) $(INCLUDES) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles $(DEBUG_FLAG)
CPPFLAGS	+= -Wall -Werror

all: $(OBJS) $(OUTPUT) $(MAP)

$(MAP): $(OUTPUT)
	$(NM) $< > $@

clean:
	rm -f $(MAP) $(OUTPUT) $(BIN) \
	$(MACHINE).lds $(OBJS)

$(OUTPUT): $(MACHINE).lds $(OBJS)
	$(CC) $(CPPFLAGS) -e __start -T $(MACHINE).lds -o $@ $(OBJS)


$(BIN): $(OUTPUT)
	$(OBJCOPY) -O binary $(OUTPUT) $(BIN)

%.o: %.S
	$(CC) $(CPPFLAGS) -I. -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) -I. -c -o $@ $<

$(MACHINE).lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) -E -P -C -o $@ $<

%: force
	$(MAKE) -C $(KERNEL_SRC) $@

force: ;

Makefile: ;

.PHONY: all clean config.mk
