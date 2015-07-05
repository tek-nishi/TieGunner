#---------------------------------------------------------------#
#   Copyright 2003 (c) Hiramatsu Kunihito All rights reserved.	#
#   E-Mail: hira@din.or.jp                                      #
#...............................................................#
#   TARGET:     ターゲット名                                    #
#   xSOURCE:    ソースファイル                                  #
#      新しいオブジェクトを追加する場合は、そのソースとなる     #
#      ファイル名を "xSOURCE" に追加して、"make depend"  と     #
#      とする、新しい従属規則は、"Makefile"の最後に追加され     #
#      る。							#
#   DFLAGS:	環境変数の設定					#
#   LIBNAME:	ライブラリーの指定				#
#   ※この「Makefile」流用する場合、「#=== ... ===#」まで設	#
#   定すれば良いようになっている。				#
#...............................................................#
#   February,   5, 2003:     Version 0.10                       #
#---------------------------------------------------------------#
TARGET	=	fpack
ASOURCE	=
CSOURCE	=	main.c	\
		hash.c	\
		misc.c

#   "makedepend" にオプションを渡せるように、"DFLAGS" には、"-D" のみ指定。
DFLAGS	=

#  ライブラリー名列挙
LIBNAME	=	

#  Warnning 制御フラグ
WNFLAGS	=	error implicit return-type switch format
#WNFLAGS	=	error implicit return-type unused switch format
# WNFLAGS	=	error all

#===============================================================#
#   include / library path, loading library.			#
#===============================================================#

#   ライブラリーのリンクの順番が重要なので注意する事。
STDLIB	=	$(addprefix -l, $(LIBNAME))

#   オブジェクトファイル名は、以下のルールによる。
OBJECTS	=	$(ASOURCE:.s=.o) $(CSOURCE:.c=.o)

#
#   Compiler, Linker Options, Resource_compiler.
#
AS	=	gcc
CC	=	gcc
CPP	=	gcc
LD	=	gcc
DEL	=	rm -f

AFLAGS	=
CFLAGS	=	-O2
PFLAGS	=
LFLAGS	=

CFLAGS +=	$(DFLAGS)

CWARNS	=	$(addprefix -W, $(WNFLAGS))

.SUFFIXES:
.SUFFIXES: .s .h .c .cpp .o

#   同名ファイルとの衝突を避けるためのターゲット指定
.PHONY: clean depend test


$(TARGET): $(OBJECTS) Makefile
	$(LD) $(LFLAGS) -o $(TARGET) $(STDLIB) $(OBJECTS) 

.s.o:
	$(AS) $(AFLAGS) -c $< -o $@

.c.o:
	$(CC) $(COPT) $(CFLAGS) $(CWARNS) -c $< -o $@

.cpp.o:
	$(CPP) $(POPT) $(PFLAGS) -c $< -o $@

#   無条件作成用エントリー
all: $(TARGET)
	make clean
	make depend

#   オブジェクトの消去用エントリー
clean:
	$(DEL) $(OBJECTS)
	$(DEL) $(TARGET)

#   従属規則作成用エントリー
depend:
	makedepend -Y -- $(DFLAGS) -- $(ASOURCE) $(CSOURCE)

#    テスト
test:
	cat test.lst | ./fpack test.pak

#   厳密な個々の従属規則

# DO NOT DELETE

main.o: default.h hash.h misc.h main.h
hash.o: default.h hash.h main.h
misc.o: default.h misc.h
