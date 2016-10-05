# need to install gsl in advance!
# > apt-get install gsl-devel

create_fir_wav : create_fir_wav.cpp
	g++ -O2 -o $@ $+ -lgsl -lgslcblas -lm
