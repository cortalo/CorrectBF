#include <iostream>
#include <fstream>
#include <set>
#include "hash_functions.h"
using namespace std;

#define STR_MAX_LEN_INPUT 8
#define STR_MAX_LEN 4

#define INPUT_NUM_BLACK 10000       //number of items forming black
#define INPUT_NUM_STREAM 100000      //number of items in stream
#define BLACK_TABLE 4000            //black table size
#define WHITE_TABLE 2000            //white table size
int bf_counters = 10000;

struct MyInput {
public:
	char key[STR_MAX_LEN_INPUT] = { 0 };
};

class MyKey {
public:
	char key[STR_MAX_LEN] = { 0 };
};

bool operator < (MyKey an, MyKey bn) {
	for (int i = 0;i < STR_MAX_LEN;i++) {
		if (bn.key[i] < an.key[i]) {
			return true;
		}
		else if (bn.key[i] > an.key[i]) {
			return false;
		}
	}
	return false;
}

bool operator > (MyKey an, MyKey bn) {
	for (int i = 0;i < STR_MAX_LEN;i++) {
		if (bn.key[i] > an.key[i]) {
			return true;
		}
		else if (bn.key[i] < an.key[i]) {
			return false;
		}
	}
	return false;
}

bool operator == (MyKey an, MyKey bn) {
	for (int i = 0;i < STR_MAX_LEN;i++) {
		if (bn.key[i] != an.key[i]) {
			return false;
		}
	}
	return true;
}


bool operator != (MyKey an, MyKey bn) {
	for (int i = 0;i < STR_MAX_LEN;i++) {
		if (bn.key[i] != an.key[i]) {
			return true;
		}
	}
	return false;
}

class BloomFilter{
public:
    int hash_number;
    int hash_seed_offset;
    int counters;
    bool *bloom_filter;
    int white_shift;
    BloomFilter(int num, int _hash_number, int _hash_seed_offset, int _white_shift){
        counters = num;
        bloom_filter = new bool[num];
        memset(bloom_filter, 0, sizeof(bloom_filter));
        hash_number = _hash_number;
        white_shift = _white_shift;
    }
    void insert(const char* key){
        for (int i = 0; i < hash_number;i++){
            unsigned index = murmur3_32(key, STR_MAX_LEN, i + hash_seed_offset) % counters;
            bloom_filter[index] = 1;
        }
    }
    bool query(const char* key){
        bool ret = 1;
        for(int i = 0; i < hash_number;i++){
            int index = murmur3_32(key, STR_MAX_LEN, i + hash_seed_offset) % counters;
            ret = ret && bloom_filter[index];
        }
        return ret;
    }

    void white(const char* key){
        for (int i = 0; i < hash_number;i++){
            unsigned index = (murmur3_32(key, STR_MAX_LEN, i + hash_seed_offset) + white_shift) % counters;
            bloom_filter[index] = 1;
        }
    }

    bool white_query(const char* key){
        bool ret = 1;
        for(int i = 0; i < hash_number;i++){
            int index = (murmur3_32(key, STR_MAX_LEN, i + hash_seed_offset) + white_shift) % counters;
            ret = ret && bloom_filter[index];
        }
        return ret;
    }

};

class HashTable{
public:
    int table_length;
    MyKey* table_begin;
    int hash_seed_offset;
    int max_shift;
    HashTable(int _table_length, int _hash_seed_offset, int _max_shift){
        table_length = _table_length;
        hash_seed_offset = _hash_seed_offset;
        table_begin = new MyKey[table_length];
        memset(table_begin, 0, sizeof(table_begin));
        max_shift = _max_shift;
    }
    void insert(const char *key){
        unsigned int index = murmur3_32(key, STR_MAX_LEN, hash_seed_offset) % table_length;
        MyKey tmp_key;
        MyKey mykey;
        memset(tmp_key.key, 0, STR_MAX_LEN);
        memcpy(mykey.key, key, STR_MAX_LEN);

        if(table_begin[index] == tmp_key){
            memcpy(table_begin[index].key, key, STR_MAX_LEN);
        }
        else{
            if(table_begin[index] == mykey){
                return;
            }
            index = (index + 1) % table_length;
            int cnt = 0;
            while(table_begin[index] != tmp_key ){
                if(table_begin[index] == mykey){
                    return;
                }
                cnt++;
                index = (index + 1) % table_length;
                if(cnt > max_shift){
                    cout <<"hash table ERROR!!!"<<endl;
                }
            }
            memcpy(table_begin[index].key, key, STR_MAX_LEN);
        }
    }

    bool query(const char *key){
        unsigned int index = murmur3_32(key, STR_MAX_LEN, hash_seed_offset) % table_length;
        MyKey tmp_key;
        MyKey mykey;
        memset(tmp_key.key, 0, STR_MAX_LEN);
        memcpy(mykey.key, key, STR_MAX_LEN);

        if(table_begin[index] == tmp_key){
            return false;
        }
        else{
            if(table_begin[index] == mykey){
                return true;
            }

            index = (index + 1) % table_length;
            int cnt = 0;
            while(table_begin[index] != tmp_key ){
                if(table_begin[index] == mykey){
                return true;
                }
                cnt++;
                index = (index + 1) % table_length;
                if(cnt > max_shift){
                    cout <<"hash table ERROR!!!"<<endl;
                }
            }
            return false;
        }
    }
};

MyInput black_input[INPUT_NUM_BLACK];
MyInput stream_input[INPUT_NUM_STREAM];
MyKey black_key[INPUT_NUM_BLACK];
MyKey stream_key[INPUT_NUM_STREAM];
HashTable black_table(BLACK_TABLE, 10, 20);
HashTable while_table(WHITE_TABLE, 10, 40);

int main(int argc, char* argv[]){

    BloomFilter bloomfilter(bf_counters, 3, 2, 2);
    set<MyKey> black_set;
    set<MyKey> true_set;
    set<MyKey> experi_set;
    set<MyKey> only_bf_set;
    fstream fin("/Users/helong/1806/data_subset/formatted00.dat", ios::in | ios::binary);
    for (int i = 0; i < INPUT_NUM_BLACK;i++) {
		fin.read((char*)(&black_input[i]), sizeof(black_input[i]));
	}
    for (int i = 0; i < INPUT_NUM_STREAM;i++){
		fin.read((char*)(&stream_input[i]), sizeof(stream_input[i]));        
    }
	fin.close();
    for (int i = 0; i < INPUT_NUM_BLACK; i++){
        memcpy(black_key[i].key, black_input[i].key, STR_MAX_LEN);
    }
    for (int i = 0; i < INPUT_NUM_STREAM; i++){
        memcpy(stream_key[i].key, stream_input[i].key, STR_MAX_LEN);
    }


    for (int i = 0; i < INPUT_NUM_BLACK;i++){
        if(black_set.find(black_key[i]) == black_set.end()){
            black_set.insert(black_key[i]);
        }
        else{
            int a = 0;
        }
        bloomfilter.insert(black_key[i].key);
        black_table.insert(black_key[i].key);
    }

    int true_num = 0;
    for(int i = 0; i < INPUT_NUM_STREAM; i++){
        if( black_set.find(stream_key[i]) == black_set.end()){
            true_num ++;
            if(true_set.find(stream_key[i]) == true_set.end()){
                true_set.insert(stream_key[i]);
            }
        }
    }

    int exp_num = 0;
    int only_bf_num = 0;
    for(int i = 0; i < INPUT_NUM_STREAM; i++){
        bool bf_query = bloomfilter.query(stream_key[i].key);
        if(!bf_query){
            exp_num ++;
            only_bf_num++;
            if(experi_set.find(stream_key[i]) == experi_set.end()){
                experi_set.insert(stream_key[i]);
            }
            if(only_bf_set.find(stream_key[i]) == only_bf_set.end()){
                only_bf_set.insert(stream_key[i]);
            }
        }
        else{
            bool bf_w_query = bloomfilter.white_query(stream_key[i].key);
            if(bf_w_query){
                bool white_query = while_table.query(stream_key[i].key);
                if(white_query){
                    exp_num ++;
                    if(experi_set.find(stream_key[i]) == experi_set.end()){
                        experi_set.insert(stream_key[i]);
                    }
                }
                else{
                    bool black_query = black_table.query(stream_key[i].key);
                    if(!black_query){
                        while_table.insert(stream_key[i].key);
                    }
                }
            }
            else{
                bool black_query = black_table.query(stream_key[i].key);
                    if(!black_query){
                        bloomfilter.white(stream_key[i].key);
                        while_table.insert(stream_key[i].key);
                    }
            }

        }
    }
    int true_size = true_set.size();
    int experi_size = experi_set.size();
    int only_bf_size = only_bf_set.size();

    int cnt0 = 0;
    int cnt1 = 0;
    for (int i = 0; i < bf_counters;i++){
        if(bloomfilter.bloom_filter[i] == 1){
            cnt1 ++;
        }
        else{
            cnt0 ++;
        }
    }
    return 0;
    
    /*
    int cnt0 = 0;
    int cnt1 = 0;
    for (int i = 0; i < bf_counters;i++){
        if(bloomfilter.bloom_filter[i] == 1){
            cnt1 ++;
        }
        else{
            cnt0 ++;
        }
    }
    cout << "0 number:" << cnt0 << endl << "1 number:" << cnt1 << endl;
    
    MyKey empty_key;
    memset(empty_key.key, 0, STR_MAX_LEN);
    int black_cnt=0;
    for (int i = 0; i < BLACK_TABLE; i++){
        if(black_table.table_begin[i] != empty_key){
            black_cnt++;
        }
    }
    cout << black_cnt<<endl;
    cout << black_set.size()<< endl;*/
}
