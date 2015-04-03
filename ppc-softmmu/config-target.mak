# Automatically generated by configure - do not modify
CONFIG_QEMU_INTERP_PREFIX="/usr/gnemul/qemu-ppc"
TARGET_ARCH=ppc
TARGET_PPC=y
TARGET_ARCH2=ppc
TARGET_BASE_ARCH=ppc
TARGET_ABI_DIR=ppc
TARGET_WORDS_BIGENDIAN=y
TARGET_PHYS_ADDR_BITS=32
CONFIG_SOFTMMU=y
LIBS+=-lutil -lncurses  -lpng -ljpeg 
HWDIR=../libhw32
TARGET_XML_FILES= /home/nkfly/qemu-0.13.0/gdb-xml/power-core.xml /home/nkfly/qemu-0.13.0/gdb-xml/power-fpu.xml /home/nkfly/qemu-0.13.0/gdb-xml/power-altivec.xml /home/nkfly/qemu-0.13.0/gdb-xml/power-spe.xml
CONFIG_SOFTFLOAT=y
CONFIG_I386_DIS=y
CONFIG_PPC_DIS=y
LDFLAGS+=
QEMU_CFLAGS+=-DHAS_AUDIO -DHAS_AUDIO_CHOICE -I$(SRC_PATH)/fpu -I$(SRC_PATH)/tcg -I$(SRC_PATH)/tcg/i386 
