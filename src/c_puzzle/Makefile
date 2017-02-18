# Makefile ( n source file n execute file version )
# 2002/12/ 4

# 日本語のコメント部分に従って修正して下さい。
# $(HOGEHOGE) というのはマクロ（一種の変数）で、
# HOGEHOGE=... で定義された内容に展開して解釈されます。
# 行頭のタブには意味がありますので、スペースに変換しないで下さい。
# できれば、最終的に日本語コメントは削除して下さい。

#	 ↓パッケージ名に書き換える
PACKAGE	= hogehoge
#	 ↓ソースファイルを書き並べる
#SRCS	= puzzle3.2.c
SRCS   := $(wildcard *.c)
PROGS	= $(SRCS:.c=)

#	 ↓パッケージに含めるファイル名を書き並べる
FILES	= README Makefile $(SRCS)
VER	= `date +%Y%m%d`


### command and flags ###
# uncomment when debugging
#DEBUG	= -ggdb -pg # -lefence

# common (*.o)
LDLIBS	= -lm

# C (*.c)
CC	= gcc
#CFLAGS	= -g -O2 -Wall $(DEBUG)
CFLAGS	= 
CPPFLAGS= -I.

# C++ (*.cc)
CXX	= g++
CXXFLAGS= -g -O2 -Wall $(DEBUG)

# Fortran77 (*.f)
FC	= f77
FFLAGS	= -Wall $(DEBUG)

# Pascal (*.p)
PC	= pc
PFLAGS	= -Wall $(DEBUG)

# etc
SHELL	= /bin/sh
RM	= rm -f
PROF	= gprof


### rules ###

.SUFFIXES:
.SUFFIXES: .o .c .cc .f .p

all: $(PROGS)

#$(PROGS): Makefile

.c:
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDLIBS)
.cc:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< -o $@ $(LDLIBS)
.f:
	$(FC) $(FFLAGS) $< -o $@ $(LDLIBS)
.p:
	$(PC) $(PFLAGS) $(CPPFLAGS) $< -o $@ $(LDLIBS)


### useful commands ###

clean:
	$(RM) $(PROGS)
	$(RM) core gmon.out *~ #*#

tar:
	@echo $(PACKAGE)-$(VER) > .package
	@$(RM) -r `cat .package`
	@mkdir `cat .package`
	@ln $(FILES) `cat .package`
	tar cvf - `cat .package` | gzip -9 > `cat .package`.tar.gz
	@$(RM) -r `cat .package` .package

zip:
	zip -9 $(PACKAGE)-$(VER).zip $(FILES)


prof: run
	$(PROF) $(PACKAGE) | less

run: all
#	 ↓サンプルデータで実行するように書き直す
#	    $(PACKAGE) は実行ファイル名に展開される
#	    行頭はタブ（スペースではない）
	./$(PACKAGE) < sample-data | less
