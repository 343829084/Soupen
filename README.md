## Yedis

Yedis is a high performance memory nosql database.

It is designed to replace redis.

Yedis means Yebangyu + redis. LOL.

## How to build

cd src/server

g++ -ltcmalloc -O2 yedis_global_info.cpp yedis_db.cpp ../ds/yedis_string.cpp ../ds/yedis_bloom_filter.cpp yedis_order.cpp yedis_epoll.cpp yedis_server.cpp -o yedis_server

Tcmalloc is highly recommended to improve the performance of yedis.

## How to use

python src/client/client.py

## Commond Supported Right Now

###BloomFilter

bfcontains key value 

//check value exists in bloom filter key or not

bfadd key value 

//add value to bloom filter key 

bfcreate key n m

//create a bloom filter named key with size n (BYTES)
//m stands for the size of set

bfdel key

//del bloom filter key

### Trie

tset key value flag

//create a trie named key and insert value to it. flag can be 0 or 1 and used to indicate case sensitive or not

tcontains key value

//check the trie named key contains value or not

tdel key

//del trie named key
## samples

###BloomFilter

bfcreate DB 1024 10 //ok

bfadd DB mysql //ok

bfadd DB redis //ok

bfadd DB yedis //ok

bfadd DB oracle //ok

bfcontains DB yedis //exist

bfcontains DB oracle //exist

bfcontains DB c++ //not exist

bfdel DB //ok

### Trie

tset db mysql 1 //ok

tset db redis 1 //ok.

tset db yedis 1 //ok

//1 means case sensitive, while 0 means not.

tcontains db yedis //exist !

tcontains db redis //exist !

tcontains db REDIS //not exist !

//Note that, the behaviour of following is undefined:

tset db mysql 1

tset db redis 0 //must be "tset db redis 1" , NOT  "tset db redis 0" 

that is to say, you should determine the flag before you want to create the trie and keep it consistent within a key.







