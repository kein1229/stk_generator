#include<iostream>
#include<fstream>
#include<vector>
#include<list>
#include<string.h>

using namespace std;

typedef unsigned int UINT;

typedef enum{
	eSUB,
	ePUSH,
	eCALL,
	eADD,
	eFRAME,
	eRET,
	eNONE,
}eTYPE;

typedef struct{
	UINT caller;
	UINT callee;
	UINT nextIns;
	int nAdd;
	int nStack;
	int nSavedEbp;
	int nCount;
	int nMax;
}DATA;

// Stack structure
typedef struct{
	UINT caller;
	UINT callee;
	UINT nextIns;
}SDATA;

typedef struct{
	SDATA sData;
	int nStack;
	int nDepth;
	string funcname;
}SINFO;

eTYPE read_type(ifstream& fin);
void read_call(ifstream& fin);
void read_add(ifstream& fin);
void read_ret(ifstream& fin);
void read_stack(ifstream& fin);
void read_sub(ifstream& fin);
void insert_data(UINT callee, UINT caller, UINT nextIns);
void print_call(char* buf);
void print_stack(char* buf);
void destory();
DATA* search_data(UINT callee, UINT caller, UINT nextIns);
void dec_stack(UINT ret);
void inc_stack(UINT callee, UINT nextIns, string& str);
