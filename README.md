## Yedis

Yedis is a high performance memory nosql database.

It is designed to replace redis.

Yedis means Yebangyu + redis. LOL.

## How to build

g++ -O2 src/server/yedis_info_manager.cpp src/server/yedis_db.cpp src/ds/yedis_string.cpp src/ds/yedis_treap.cpp src/ds/yedis_bloom_filter.cpp src/server/yedis_order.cpp src/server/yedis_epoll.cpp src/server/yedis_server.cpp -o yedis_server

src/server/yedis_server


## How to use

python src/client/yedis_client.py

## Feature Supported Right Now

### DB operation

select 12 //ok.select 12th db

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

tdel db //ok. delete the trie db

//Note that, the behaviour of following is undefined:

tset db mysql 1

tset db redis 0 //must be "tset db redis 1" , NOT  "tset db redis 0" 

that is to say, you should determine the flag before you want to create the trie and keep it consistent within a key.

## Truth

### server

![server](http://7xnljs.com1.z0.glb.clouddn.com/server.png)

### client

![client](http://7xnljs.com1.z0.glb.clouddn.com/client.png)









