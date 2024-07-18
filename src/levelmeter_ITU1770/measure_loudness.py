#! /bin/env python3

# https://nettyukobo.com/loudness_lufs/

import numpy as np
import soundfile as sf
import scipy.signal as sg
import resampy

# 入力WAVデータ
wav_in_name = "input.wav"

# WAV を読み込む
x, fs = sf.read(wav_in_name, always_2d=True)

# リサンプリング
x_r = resampy.resample(x, fs, 48000, axis=0)

# フィルタ係数
a = np.array([1.0, -1.69065929318241, 0.73248077421585])
b = np.array([1.53512485958697, -2.69169618940638, 1.19839281085285])
c = np.array([1.0, -1.99004745483398, 0.99007225036621])
d = np.array([1.0, -2.0, 1.0])

# 定数
zmin = 1e-10               # 2乗平均の最小
block_len = int(48000 * 0.4)  # ブロック長 400ms
slide     = int(48000 * 0.1)  # スライド長 100ms
length  = x_r.shape[0]     # 信号の長さ
channel = x_r.shape[1]     # チャンネル数

# LR ch の重みづけ
if channel == 1:
    G_LR = [1.0]
elif channel == 2:
    G_LR = [1.0, 1.0]

# K特性フィルタを畳み込む
y = sg.lfilter(b=b, a=a, x=x_r, axis=0)
y = sg.lfilter(b=d, a=c, x=y,   axis=0)

# ゲーティングブロックごとに2乗平均を求める
n_block = (length - block_len) // slide + 1 # ブロック数
z = np.zeros((n_block, channel))
for j in range(n_block):
    z[j,:] = np.mean(y[j*slide:j*slide+block_len,:]**2, axis=0) 

# 絶対ゲーテイング
l_tmp = np.sum(np.maximum(zmin, z*G_LR), axis=1) 
l = -0.691 + 10.0 * np.log10(l_tmp)
abs_gate = (l > -70.0) # 絶対閾値を超える要素は True

# 相対ゲーテイング
z_abs = np.zeros_like(z)  # z と同じ形のNumpy配列作成
for i in range(channel):  # 絶対閾値以下の要素は 0 にする
    z_abs[:,i] = z[:,i] * abs_gate
z_mean_r = np.sum(z_abs, axis=0)/np.sum(abs_gate)
rel_thd_tmp = np.sum(z_mean_r*G_LR)
rel_thd = -0.691 -10.0 + 10.0 * np.log10(rel_thd_tmp) # 相対閾値
rel_gate = (l > rel_thd) # 相対閾値を超える要素は True

# 平均ラウドネス値を算出
gate = abs_gate & rel_gate
z_gate = np.zeros_like(z)  # z と同じ形のNumpy配列作成
for i in range(channel):   # 絶対閾値と相対閾値以下の要素は 0 にする
    z_gate[:,i] = z[:,i] * gate
z_mean_g = np.sum(z_gate, axis=0)/np.sum(gate)
Loudness_tmp = np.sum(z_mean_g*G_LR)
Loudness = -0.691 + 10.0 * np.log10(Loudness_tmp)

# ラウドネスを表示
print("Loudness: {0:.2f} [LUFS]".format(Loudness))
