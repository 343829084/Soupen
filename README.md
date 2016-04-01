## Yedis

Yedis is a high performance memory nosql database.

It is designed to replace redis.

Yedis means Yebangyu + redis. LOL.

## How to build

cd src/server

g++ -O2 yedis_global_info.cpp yedis_db.cpp ../ds/yedis_string.cpp ../ds/yedis_bloom_filter.cpp yedis_order.cpp yedis_epoll.cpp yedis_server.cpp -o yedis_server

Tcmalloc is highly recommended to improve the performance of yedis.

## How to use

python src/client/client.py

## Feature Supported Right Now

### db operation

select 12 //ok . change db. select 12th db

###BloomFilter

bfcreate DB 1024 10 //ok

bfadd DB mysql //ok

bfadd DB redis //ok

bfadd DB yedis //ok

bfadd DB oracle //ok

bfcontains DB yedis //true

bfcontains DB oracle //true

bfcontains DB c++ //false

bfdel DB //ok

### Trie

tset db mysql 1 //ok

tset db redis 1 //ok.

tset db yedis 1 //ok

//1 means case sensitive, while 0 means not.

tcontains db yedis //true

tcontains db redis //true

tcontains db REDIS //true

//Note that, the behaviour of following is undefined:

tset db mysql 1

tset db redis 0 //must be "tset db redis 1" , NOT  "tset db redis 0" 

that is to say, you should determine the flag before you want to create the trie and keep it consistent within a key.

## Truth

### server

![server](http://7xnljs.com1.z0.glb.clouddn.com/server.png)

### client

![client](http://7xnljs.com1.z0.glb.clouddn.com/client.png)









