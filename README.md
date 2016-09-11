组员 1452656 吕金华
    1452661 易思琦

项目说明 
实现为： B级，修改或重新实现参考源码的一个或多个模块
            具体为 进程调度模块 涉及三个状态和五个优先级，多级队列调度
       C级，在参考源码上实现系统级应用
            具体为，控制台 控制台能支持help，clear，exit，ps等简单的命令

       D级，在参考源码上实现一个用户级应用
            具体为，计算器 计算器能实现四则运算，即int数据的加减乘除

项目环境

Linux Ubuntu 16.04LTS, 
bochs 2.6.8

使用方法

确保你已经装了Ubuntu 和 Bochs
在terminal里面运行下面的命令完成配置
$ sudo  apt-get update 
$ sudo  apt-get install build-essential 
$ sudo  apt-get install xorg-dev                                        
$ sudo  apt-get install bison   
$ sudo  apt-get install libgtk2.0-dev
$ sudo  apt-get install nasm
$ sudo  apt-get install vgabios
将项目clone到本地,首先打开到目标文件夹下,然后运行:
$ git clone https://github.com/happyfarmergo/ShabbyOS.git
$ cd ShabbyOS/
$ sudo make image
$ bochs -f bochsrc.bxrc
$ 回车再输入 c 进入
项目分工

我主要负责了项目的整体的初始化，并将进程调度算法改为了多级队列调度，以及写了控制台的系统级应用。
易思琦主要负责写了一个计算器的用户级应用，并且负责了我们小组的文档，ppt等工作。
大家都对书上的源码做了深入的了解，并且在源码的基础上做出了相应的改动，每个人都非常的努力。

Summary:

感谢队友的辛苦付出，同时也受到了一些同学、学长的帮助，在此一并感谢。





Github项目托管地址:https://github.com/happyfarmergo/ShabbyOS

