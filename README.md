#Shabby OS

##项目成员
  * 1452656 吕金华
  * 1452662 易思琦

##项目说明
* 重新实现了进程调度模块，采用多级队列调度方式
  - 进程有五种状态：创生，就绪，运行，等待，和消亡。
  - 每个进程属于IDLE，LOW，MEDIUM，HIGH，和REALTIME这五种优先级别之一。
  - 在多个优先级别的队列中选择优先级别最大的进程调度。

* 实现了终端这一系统级应用和计算器这一用户级应用
  - 终端支持clear，ps，exit等简单的命令
  - 计算器能够进行十以内的四则运算

##项目环境

  * Ubuntu 16.04 LTS(x86)
  * bochs(2.6.8)

## 使用方法

  * 确保你已经装了Ubuntu 和 Bochs

  * 在terminal里面运行下面的命令完成配置

    $ sudo  apt-get install build-essential nasm bochs bochs-x vgabios
  * 将项目clone到本地,首先打开到目标文件夹下,然后运行:

    $ git clone https://github.com/happyfarmergo/ShabbyOS.git

    $ cd ShabbyOS/

    $ sudo make image

    $ bochs -f bochsrc

    $ 回车再输入 c 进入

## 项目分工

  * 易思琦主要负责写了一个计算器的用户级应用，并且负责了我们小组的文档，ppt等工作。

  * 吕金华主要负责了项目的整体的初始化，并将进程调度算法改为了多级队列调度，以及写了控制台的系统级应用。

  * 大家都对书上的源码做了深入的了解，并且在源码的基础上做出了相应的改动，每个人都非常的努力。


### Github项目托管地址:
https://github.com/happyfarmergo/ShabbyOS

