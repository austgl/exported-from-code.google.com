HTTPCWS ��һ�����HTTPЭ������ķִ�ϵͳ��

HTTPCWS������̣�
ע��httpcwsΪ��̬����Ķ����ư汾��������룬����ֱ��ͨ��./httpcwsִ�С��������Ҫ�ֹ����룬�밴�����·������У������飩��

1�������ذ�װlibevent��
wget http://www.monkey.org/~provos/libevent-1.4.11-stable.tar.gz
tar zxvf libevent-1.4.11-stable.tar.gz
cd libevent-1.4.11-stable/
./configure --prefix=/usr
make && make install
cd ../
echo "/usr/lib" > /etc/ld.so.conf.d/libevent.conf
/sbin/ldconfig

2���ٵ�http://ictclas.org/Down_share.html���ء�ICTCLAS2009�������İ桱-��Linux_C_64��������ѹ���ICTCLAS30.h��libICTCLAS30.a�ļ��ŵ�httpcws��ICTCLASĿ¼�С�

3������HTTOCWS��
��̬���룺
g++ -o httpcws httpcws.cpp -levent -L./ICTCLAS -lICTCLAS30 -Wall -Wunused -O3 -DOS_LINUX

��̬���룺
g++ -o httpcws httpcws.cpp -levent -L./ICTCLAS -lICTCLAS30 -Wall -Wunused -O3 -DOS_LINUX -static-libgcc -pthread -lrt -static


������������ʣ�
http://code.google.com/p/httpcws
http://blog.s135.com