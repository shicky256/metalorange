#
#������ makefile �̗��p�����
# 1. ���̃t�@�C�����T���v���̃f�B���N�g���ɃR�s�[���Ă��������B
# 2. �f�B���N�g���̐ݒ�𒲐����Ă��������B
#    �}�N�� GCC, SEGALIB, SEGASMP ���`���Ă��������B
# 3. �R���t�B�M�����[�V�����t�@�C�� sample.cfg ���쐬���Ă��������B
#    �}�N�� TARGET �� OBJS ���`���Ă��������B
# 4. make �����s���ăT���v���v���O�������쐬���Ă��������B
#

#
#�����̃t�@�C���Ŏg���Ă���}�N���̐���
#  CC           �b�R���p�C�����w�肵�܂��B�i���s�\�Ȍ`�ŋL�q���Ă��������j
#  CFLAGS       �b�R���p�C���̃I�v�V�������w�肵�܂��B
#  AS           �A�Z���u�����w�肵�܂��B�i���s�\�Ȍ`�ŋL�q���Ă��������j
#  ASFLAGS      �A�Z���u���̃I�v�V�������w�肵�܂��B
#  LDFLAGS      �����J�̃I�v�V�������w�肵�܂��B
#  SATURN       �T�^�[���p�̃z�[���f�B���N�g�����w�肵�܂��B
#  GCC          �f�m�t�̂���f�B���N�g�����w�肵�܂��B
#  SEGASMP      �r�a�k�̃T���v���̂���f�B���N�g�����w�肵�܂��B
#  SEGALIB      �r�a�k�̃��C�u�����̂���f�B���N�g�����w�肵�܂��B
#  LOCATE_FILE  �������̃��P�[�V�������L�q�����t�@�C���ł��B
#  ENTRY_POINT  �T���v���v���O�����̎��s�J�n�A�h���X���w�肵�܂��B
#  LIBS         �����N���郉�C�u�������w�肵�܂��B
#  
#  �i�R���t�B�M�����[�V�����t�@�C�� sample.cfg �̒��Œ�`���܂��j
#  TARGET       �T���v���v���O�����̖��O���w�肵�܂��B
#               �i�t�@�C���̃v���C�}���l�[���ɂȂ�܂��j
#  OBJS         �����N����I�u�W�F�N�g�t�@�C�����w�肵�܂��B
#
ROOT_DIR = c:/saturn
SATURN  = $(ROOT_DIR)/sbl
CDTOOLS = $(ROOT_DIR)/cdtools
MEDNAFEN = $(ROOT_DIR)/mednafen/mednafen.exe
SATBUG = $(ROOT_DIR)/orange/tools/satbug/bin/Debug/satbug.exe

CDDIR = cd
OUTDIR = out
SEGASMP = $(SATURN)/segasmp
SEGALIB = $(SATURN)/segalib

CC = sh-elf-gcc
AS = sh-elf-as
OBJCOPY = sh-elf-objcopy
ISO = $(CDTOOLS)/mkisofs

CFLAGS  = -g -O2 -Wall -Werror -std=gnu99 -m2 -DMODEL_S -I$(SEGALIB)/include -I./smpclib
ASFLAGS =
LDFLAGS = -T $(LOCATE_FILE) -e $(ENTRY_POINT) -nostartfiles
ISOFLAGS = -sysid "SEGA SATURN" -volid "SaturnApp" -volset "SaturnApp" -publisher "SEGA ENTERPRISES, LTD." -preparer "SEGA ENTERPRISES, LTD." \
 -appid "SaturnApp" -abstract "$(CDTOOLS)/ABS.TXT" -copyright "$(CDTOOLS)/CPY.TXT" -biblio "$(CDTOOLS)/BIB.TXT" -generic-boot "$(CDTOOLS)/ip_gnu.bin" -full-iso9660-filenames

LOCATE_FILE = lnk_elf.x
ENTRY_POINT = _entry
CONFIG_FILE = sample.cfg

LIBS= $(SEGALIB)/lib/libsat.a

include	$(CONFIG_FILE)

.SUFFIXES:
.SUFFIXES: .cof .o .src .c

all: $(OUTDIR)/$(TARGET).iso

run: $(OUTDIR)/$(TARGET).iso
	$(MEDNAFEN) $(OUTDIR)/$(TARGET).cue

devcart: $(OUTDIR)/$(TARGET).iso
	$(SATBUG) -x $(TARGET).bin 0x6010000 -s $(CDDIR)\\

clean:
	rm *.o
	rm *.elf
	rm *.bin

$(OUTDIR)/$(TARGET).iso: $(TARGET).bin
	cp $< $(CDDIR)/0.bin
	$(ISO) $(ISOFLAGS) -o $@ $(CDDIR)

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).elf:	$(OBJS)
	$(CC) $(LDFLAGS) $(_LDFLAGS) -o $@ -Xlinker -Map -Xlinker $(TARGET).map $(OBJS) $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $(_CFLAGS) -o $@ $<

%.o: %.s
	$(AS) $< $(ASFLAGS) $(_ASFLAGS) -o $@



