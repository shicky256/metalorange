#
#■この makefile の利用手引き
# 1. このファイルをサンプルのディレクトリにコピーしてください。
# 2. ディレクトリの設定を調整してください。
#    マクロ GCC, SEGALIB, SEGASMP を定義してください。
# 3. コンフィギュレーションファイル sample.cfg を作成してください。
#    マクロ TARGET と OBJS を定義してください。
# 4. make を実行してサンプルプログラムを作成してください。
#

#
#■このファイルで使われているマクロの説明
#  CC           Ｃコンパイラを指定します。（実行可能な形で記述してください）
#  CFLAGS       Ｃコンパイラのオプションを指定します。
#  AS           アセンブラを指定します。（実行可能な形で記述してください）
#  ASFLAGS      アセンブラのオプションを指定します。
#  LDFLAGS      リンカのオプションを指定します。
#  SATURN       サターン用のホームディレクトリを指定します。
#  GCC          ＧＮＵのあるディレクトリを指定します。
#  SEGASMP      ＳＢＬのサンプルのあるディレクトリを指定します。
#  SEGALIB      ＳＢＬのライブラリのあるディレクトリを指定します。
#  LOCATE_FILE  メモリのロケーションを記述したファイルです。
#  ENTRY_POINT  サンプルプログラムの実行開始アドレスを指定します。
#  LIBS         リンクするライブラリを指定します。
#  
#  （コンフィギュレーションファイル sample.cfg の中で定義します）
#  TARGET       サンプルプログラムの名前を指定します。
#               （ファイルのプライマリネームになります）
#  OBJS         リンクするオブジェクトファイルを指定します。
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



