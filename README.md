midpoint_displacement_algorithm
===============================

3Dメッシュに対して中点変位法を行うプログラムです。
Windows上でOpenGL使って書いています。
でもWindows依存なコードは書いていないはず。

操作方法
n: 分割数を増やす
p: 分割数を減らす
十字キーの左右: カメラ回転

分割数が1増えるとポリゴンの数が4倍になるので、ある程度分割すると処理が一気に重くなります。

0分割
![div0](readmefile/div0.png)

1分割
![div0](readmefile/div1.png)

3分割
![div0](readmefile/div3.png)

4分割
![div0](readmefile/div4.png)

6分割
![div0](readmefile/div6.png)

8分割
![div0](readmefile/div8.png)
