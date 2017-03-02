# Makefile ( n source file n execute file version )
# 2002/12/ 4

# ���ܸ�Υ�������ʬ�˽��äƽ������Ʋ�������
# $(HOGEHOGE) �Ȥ����Τϥޥ����ʰ����ѿ��ˤǡ�
# HOGEHOGE=... ��������줿���Ƥ�Ÿ�����Ʋ�ᤵ��ޤ���
# ��Ƭ�Υ��֤ˤϰ�̣������ޤ��Τǡ����ڡ������Ѵ����ʤ��ǲ�������
# �Ǥ���С��ǽ�Ū�����ܸ쥳���ȤϺ�����Ʋ�������

#	 ���ѥå�����̾�˽񤭴�����
PACKAGE	= hogehoge
#	 ���������ե��������¤٤�
#SRCS	= puzzle3.2.c
SRCS   := $(wildcard *.c)
PROGS	= $(SRCS:.c=)

#	 ���ѥå������˴ޤ��ե�����̾����¤٤�
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
#	 ������ץ�ǡ����Ǽ¹Ԥ���褦�˽�ľ��
#	    $(PACKAGE) �ϼ¹ԥե�����̾��Ÿ�������
#	    ��Ƭ�ϥ��֡ʥ��ڡ����ǤϤʤ���
	./$(PACKAGE) < sample-data | less