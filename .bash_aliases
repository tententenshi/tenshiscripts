# A couple of default aliases.

alias mv='mv -i'
alias rm='rm -i'
alias cp='cp -i'

alias la='ls -FA --color=auto'
alias lt='ls -FAt --color=auto'
function ll () {
	ls -lagF --color=auto $* | less
}
alias ls='ls -F --color=auto'
alias l='ls -lagF --full-time --color=auto | less'

cd () {
	builtin cd "$*" ; echo $PWD
}
alias back='cd "$OLDPWD"'

function e () {
	$EDITOR $*
}

alias h='history | less'
alias c=clear

alias du='du --max-depth=1'

function jhd {
	 hexdump -C $* | less
}

function psg () {
	ps ax | grep $* | grep -v grep
}

function sg () {
	tar_path=$1; shift
	pattern=`echo $* | sed 's/\\\\t/	/g' | sed 's/\\\\s/[ 	]/g' | sed 's/\\\\S/[^ 	]/g' | sed 's/\\\\d/[0-9]/g' | sed 's/\\\\D/[^0-9]/g'`
	command find $tar_path -follow \( \( -name "*~" -o -name "*.svn*" -o -name "*.htm*" -o -name "*.crf*" \) -o \( -name "*.[csShx]*" -or -name "*.asp" -or -name "*.e[ah]" -or -name "*.v*[ah]" -or -name "*.fda" -or -name "*.blk" -or -name "*.inc" -or -name "*.preprocess" -or -name "Makefile*" -or -name "make.com" \) -type f -print0 \) |xargs -0 grep -aE "$pattern" | less -p "$pattern"
}
function Sg () {
	tar_path=$1; shift
	pattern=`echo $* | sed 's/\\\\t/	/g' | sed 's/\\\\s/[ 	]/g' | sed 's/\\\\S/[^ 	]/g' | sed 's/\\\\d/[0-9]/g' | sed 's/\\\\D/[^0-9]/g'`
	command find $tar_path -follow \( \( -name "*~" -o -name "*.svn*" -o -name "*.htm*" -o -name "*.crf*" \) -o \( -name "*.[csShx]*" -or -name "*.asp" -or -name "*.e[ah]" -or -name "*.v*[ah]" -or -name "*.fda" -or -name "*.blk" -or -name "*.inc" -or -name "*.preprocess" -or -name "Makefile*" -or -name "make.com" \) -type f -print0 \) |xargs -0 grep -aiE "$pattern" | less -i -p "$pattern"
}

function sg_list () {
	tar_path=$1; shift
	pattern=`echo $* | sed 's/\\\\t/	/g' | sed 's/\\\\s/[ 	]/g' | sed 's/\\\\S/[^ 	]/g' | sed 's/\\\\d/[0-9]/g' | sed 's/\\\\D/[^0-9]/g'`
	command find $tar_path -follow \( \( -name "*~" -o -name "*.svn*" -o -name "*.htm*" -o -name "*.crf*" \) -o \( -name "*.[csShx]*" -or -name "*.asp" -or -name "*.e[ah]" -or -name "*.v*[ah]" -or -name "*.fda" -or -name "*.blk" -or -name "*.inc" -or -name "*.preprocess" -or -name "Makefile*" -or -name "make.com" \) -type f -print0 \) |xargs -0 grep -alE "$pattern"
}
function Sg_list () {
	tar_path=$1; shift
	pattern=`echo $* | sed 's/\\\\t/	/g' | sed 's/\\\\s/[ 	]/g' | sed 's/\\\\S/[^ 	]/g' | sed 's/\\\\d/[0-9]/g' | sed 's/\\\\D/[^0-9]/g'`
	command find $tar_path -follow \( \( -name "*~" -o -name "*.svn*" -o -name "*.htm*" -o -name "*.crf*" \) -o \( -name "*.[csShx]*" -or -name "*.asp" -or -name "*.e[ah]" -or -name "*.v*[ah]" -or -name "*.fda" -or -name "*.blk" -or -name "*.inc" -or -name "*.preprocess" -or -name "Makefile*" -or -name "make.com" \) -type f -print0 \) |xargs -0 grep -ailE "$pattern"
}

function calcf() {
	perl -e "
		use POSIX qw(floor ceil);
		use strict;
		sub pi{4*atan2(1,1)};
		sub tan{sin(\$_[0])/cos(\$_[0])};
		sub log10{log(\$_[0])/log(10)};
		sub my_hex_float {
			my (\$str)=@_;
			if (\$str =~ /([+-]?0[xX][0-9a-fA-F]+)(\.[0-9a-fA-F]*)?[pP]([+-]?\d+)/) {
				my (\$int_val, \$frac_val, \$shift) = (\$1, \$2, \$3);
				\$frac_val =~ s/\.//;
				my \$frac_digit = length(\$frac_val);
				\$shift -= \$frac_digit * 4;
				my \$shift_val = 2**\$shift;
				my \$val = eval(\$int_val . \$frac_val) * \$shift_val;
				return \$val;
			} else {
				return 0;
			}
		};
		my \$str=\"$*\";
		while (\$str =~ /(.*)([+-]?0[xX][0-9a-fA-F]+)(\.[0-9a-fA-F]*)?([pP][+-]?\d+)(.*)/) {
			my (\$pre, \$hex_float_val, \$post) = (\$1, \$2 . \$3 . \$4, \$5);
			\$str = \$pre . my_hex_float(\$hex_float_val) . \$post;
		}
		my \$val=eval(\$str);
		printf \"%15.12f\n\", \$val;
	"
}
function calcd() {
	CUR_VAL=`calcf "$*"`;
	perl -e "printf \"%d\n\", ($CUR_VAL>=0)?($CUR_VAL+0.5):($CUR_VAL-0.5);"
}
function calcx() {
	CUR_VAL=`calcf "$*"`;
	perl -e "printf \"0x%x\n\", ($CUR_VAL>=0)?($CUR_VAL+0.5):($CUR_VAL-0.5);"
}

function overwrite() {
	case $# in
	0|1)	echo 'Usage: overwrite file cmd [args]' 1>&2
		;;
	*)	
		file=$1; shift
		new=/tmp/overwr1.$$; old=/tmp/overwr2.$$
		trap 'rm -f $new $old; exit 1' 1 2 15		# clean up files

		if "$@" >$new					# collect input
		then
			command cp $file $old			# save original file
			trap '' 1 2 15				# ignore signals
			command cp $new $file
		else
			echo "overwrite: $1 failed, $file unchanged" 1>&2
			exit 1
		fi
		rm -f $new $old
	esac
}

function replace() {
	case $# in
	0|1|2)	echo 'Usage: replace str1 str2 files' 1>&2
		;;
	*)	
		left="$1"; right="$2"; shift; shift;

		for i
		do
			overwrite $i sed -b "s@$left@$right@g" $i
		done
	esac
}

function dclean() {
	find $* -name "*.d" -print0 | xargs -0 rm
}


#find ()
#{
#	command find $* -follow
#}
