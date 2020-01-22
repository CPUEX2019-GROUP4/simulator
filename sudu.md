## 概要  
シミュレータプログラムの速度を測定する。  
測定環境  
```
OS: Arch Linux  
Kernel: x86_64 Linux 5.4.12-arch1-1  
Shell: zsh 5.7.1  
CPU: Intel Core i7-6700 @ 8x 4GHz [20.0°C]  
GCC version: 9.2.0  
```
実行時間は毎回変わる。
目安として、8.3 sに対して8.2~8.4s代は普通に起こり得る。

## 実験日 1/22
### 0c111c6
`./test.sh sudu.s contest.bin 1`の結果:
```
total executed instructions: 2,183,261,623
real  8.262s
user  8.202s
sys   0.057s
```

### 408f815
`./test.sh sudu.s contest.bin 1`の結果:
```
total executed instructions: 2,183,261,623
real  8.442s
user  8.386s
sys   0.053s
```

### b53ce41
`./test.sh sudu.s contest.bin 1`の結果:
```
total executed instructions: 2,183,261,623
real  20.111s
user  20.061s
sys   0.041s
```
このコミットでは`get_opcode()`や`get_rd()`といった、
32bitの命令からオペコード・オペランドを取り出す関数を、ファイル分割
してみた。
これは毎回実行されているから、2 billion * 4回くらい(opcode, rd, ra, rb)
全体で実行されていることになる。
やはり、それだけ実行されていると10秒は変わるということだ。
分割コンパイルと動的リンクとは違う方法でファイル分割するプログラムを
書こうというモチベーションが生まれた。
