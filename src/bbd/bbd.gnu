set samples 10000
set xrange[0.2:4]

Max=2.0
Min=0.5
Delay=0.2

f(x)=1/((1/Max-1/Min)*x+1/Min)	# reciprocal
#f(x)=(Max-Min)*x+Min	# linear

tri(x)=abs(2*(x/2-int(x/2))-1)

read_spead(x)=f(tri(x))
write_spead(x)=f(tri(x-Delay/read_spead(x)))

plot read_spead(x), write_spead(x), read_spead(x)/write_spead(x)



# read_spead(t)=f(t) としたとき、
# write_spead(t)=f(t - D/read_spead(t)) = f(t - D/f(t))  : ただし、遅延量を D とする。
# ピッチ変動 Pitch(t) = read_spead(t)/write_spead(t) = f(t) / f(t - D/f(t)) であらわされる。
# それが 一定のピッチ変動 R になるとしたとき、
# R = f(t) / f(t - D/f(t))
# これを、微分方程式で解く。
#
# R * {f(t - D/f(t))}' = f'(t)
# R * f'(t) * (t - D/f(t))' = f'(t)
# R * (t - D/f(t))' = 1
# R * (1 + D/(f(t)**2) * f'(t)) = 1
#
# 以下は Maxima コマンド
# > diff(t-D/f(t), t,1);
# > ode2(R*(1+(D/f(t)^2*('diff(f(t),t,1)))) = 1, f(t), t);
# --> (D*R)/((R-1)*f(t))=t+%c
# となる。一気に解いてほしいが、なかなか。
