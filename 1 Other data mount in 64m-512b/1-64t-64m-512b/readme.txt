1mycreateblock-4disks.cpp
4块硬盘阵列用4个线程去写，只管生成数据

2mod-4disks.cpp
使用GMP库接口生成一个模数

3base-4disks.cpp
使用GMP库接口生成一个底数

4myclient-4disks.cpp
生成每个数据文件的标签，四块硬盘，由于磁盘IO速度和计算机时钟的影响，分若干个线程计算，根据不断试验，找出性能最优的线程数

5myauditor-4disks.cpp
审计者生成一系列随机数

6server-4disks.cpp
服务器生成一系列数

7ssort.cpp
服务器生成对比数据

8vsort.cpp
审计者生成对比数据

9mylocation.cpp
对比服务器和审计者分别生成的对比数据是否一样，不过现在是线性对比，不是二分的
