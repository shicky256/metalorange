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

CC = sh-coff-gcc
AS = gasm

CFLAGS  = -g -O2 -Wall -std=gnu99 -m2 -DMODEL_S -I$(GCC)/include -I$(SEGALIB)/include -I$(SEGASMP)/per/smpclib
ASFLAGS =
LDFLAGS = -T $(LOCATE_FILE) -e $(ENTRY_POINT) -nostartfiles

SATURN  = ../sbl6
GCC     = c:/saturn/SH_COFF/sh-coff
SEGASMP = $(SATURN)/segasmp
SEGALIB = $(SATURN)/segalib

LOCATE_FILE = saturn.lnk
ENTRY_POINT = START
CONFIG_FILE = sample.cfg

LIBS=	$(SEGALIB)/lib/sega_int.a\
	$(SEGALIB)/lib/sega_spr.a\
	$(SEGALIB)/lib/sega_scl.a\
	$(SEGALIB)/lib/sega_mth.a\
	$(SEGALIB)/lib/sega_cdc.a\
	$(SEGALIB)/lib/sega_adp.a\
	$(SEGALIB)/lib/sega_per.a\
	$(SEGALIB)/lib/sega_sat.a

include	$(CONFIG_FILE)

.SUFFIXES:
.SUFFIXES: .cof .o .src .c

all: $(TARGET).cof $(TARGET).bin

$(TARGET).cof:	$(OBJS)
	$(CC) $(LDFLAGS) $(_LDFLAGS) -o $(TARGET).cof -Xlinker -Map -Xlinker $(TARGET).map $(OBJS) $(LIBS)

$(TARGET).bin:	$(OBJS)
	sh-coff-objcopy -O binary $(TARGET).cof out.bin

%.o: %.c
	$(CC) -c $(CFLAGS) $(_CFLAGS) -o $@ $<

%.o: %.src
	cmd //c $(AS) $< $(ASFLAGS) $(_ASFLAGS) -o $@



