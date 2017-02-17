set samples 10000
set xrange[0.2:4]

Max=2.0
Min=0.5
Delay=0.2

f(x)=1/((1/Max-1/Min)*x+1/Min)	# reciprocal
#f(x)=Min*exp(log(Max/Min)*x)	# exponential
#f(x)=(Max-Min)*x+Min	# linear

tri(x)=abs(2*(x/2-int(x/2))-1)

read_spead(x)=f(tri(x))
write_spead(x)=f(tri(x-Delay/read_spead(x)))

plot read_spead(x), write_spead(x), read_spead(x)/write_spead(x)



# read_spead(t)=f(t) �Ƃ����Ƃ��A
# write_spead(t)=f(t - D/read_spead(t)) = f(t - D/f(t))  : �������A�x���ʂ� D �Ƃ���B
# �s�b�`�ϓ� Pitch(t) = read_spead(t)/write_spead(t) = f(t) / f(t - D/f(t)) �ł���킳���B
# ���ꂪ ���̃s�b�`�ϓ� R �ɂȂ�Ƃ����Ƃ��A
# R = f(t) / f(t - D/f(t))
# ������A�����������ŉ����B
#
# R * {f(t - D/f(t))}' = f'(t)
# R * f'(t) * (t - D/f(t))' = f'(t)
# R * (t - D/f(t))' = 1
# R * (1 + D/(f(t)**2) * f'(t)) = 1
#
# �ȉ��� Maxima �R�}���h
# > diff(t-D/f(t), t,1);
# > ode2(R*(1+(D/f(t)^2*('diff(f(t),t,1)))) = 1, f(t), t);
# --> (D*R)/((R-1)*f(t))=t+%c
# �ƂȂ�B��C�ɉ����Ăق������A�Ȃ��Ȃ��B


# ��̉�͂ł́A�f�B���C���́A�ǂݏo�����̃T���v�����O���g���ɂ݈̂ˑ����ĐL�яk�݂���Ɖ��肵�Ă���B
# ���A���ۂɂ́A���W�����[�V�����ɂ���č��ꍏ�ƕω����Ă���B
# ���W�����[�V�����̕ϓ��������A�f�B���C�������\��������΁A
# �u�f�B���C���́A�ǂݏo�����̃T���v�����O���g���ɂ݈̂ˑ����Ă���v�Ɖ��肵�Ă� �卷�Ȃ����A
# ���W�����[�V�����̎����ƃf�B���C�����߂��Ȃ��Ă���ƁA���̉���͓K�؂łȂ��Ȃ��Ă���B

# �f�B���C���̓��W�����[�V�����Ɋ�炸��肾�Ɖ��肵���ꍇ�̃s�b�`�ϓ��͂��̂悤�ɂȂ�B


#write_spead(x)=f(tri(x-Delay))
#plot read_spead(x), write_spead(x), read_spead(x)/write_spead(x)
