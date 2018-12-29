# RoboSys2018-Kadai
+ これは千葉工業大学 先進工学部 未来ロボティクス学科の学部3年の授業ロボットシステム学の課題です。
## 実施内容
+ RaspberryPiのGPIOを8ピン使用してchar型のデータをLEDで表示する。
+ もう一つのRaspberryPiでLEDで表示したchar型のデータを読み込みコンソールで表示する。
## 実施結果
+ [YouTube](https://www.youtube.com/watch?v=t3ZmyUtkEA0)
## 環境
+ Raspberry Pi 3 Model B Plus Rev 1.3 (BCM2835)
+ Raspbian GNU/Linux 9.6 (stretch) armv7l
## 参考
+ [レジスタマップなど](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf)
+ [GPIOの使用法など](https://www.ei.tohoku.ac.jp/xkozima/lab/raspTutorial3.html)
## 実行方法
### Send
```Bash
cd linux_module
./reload.bash
./while.bash
```
### Listener
```Bash
cd linux_module
./reload.bash
cat /dev/robosys_device0
```
