#include "graphgen.h"
#define bit 0
#define LOG 0
#define RET_SIZE 0
#define EBP_SIZE 0
#define DEPTH 1
#define ALL 0
#define CALL 1

vector<SDATA*> stack;
vector<SINFO*> vecStack;

list<DATA*> listdata;
SDATA prev_call;
list<eTYPE> listType;
int nDepth = 0;
FILE* pFDepth;


int main(int argc, char* argv[])
{
	ifstream fin;
	
	char buf[256] = {0,};
	sprintf(buf,"%s.out",argv[1]);
	fin.open(buf);

	sprintf(buf,"%s_depth.out",argv[1]);
	if( !(pFDepth = fopen(buf,"w")) )
	{
		printf("File Open Failed : %s\n",buf);
		return 0;
	}
	

	if(!fin.is_open())
	{
		cout << argv[1] << " File open failed!!" << endl;
		return 0;
	}

	while(!fin.eof())
	{
		eTYPE eType = read_type(fin);
		switch(eType)
		{
		case eCALL:
			read_call(fin);
			listType.push_back(eType);
			break;
		case eADD:
			read_add(fin);
			break;
		case eSUB:
			read_sub(fin);
			break;
		case eFRAME:
			read_stack(fin);
			break;
		case eRET:
			read_ret(fin);
			listType.push_back(eType);
			break;
		}
	}

	print_call(argv[1]);

#if 1 
	fin.clear();
	fin.seekg(0, ios::beg);
	stack.clear();

	SINFO* pNew = new SINFO;
	memset(pNew,0,sizeof(SINFO));
	vecStack.push_back(pNew);
	
	while(!fin.eof())
	{
		eTYPE eType =read_type(fin);
		switch(eType)
		{
		case eCALL:
			{
				UINT callee, ip, nextIns, caller = 0;
				string str;
				fin >> hex >> ip >> callee >> nextIns;
				fin >> str;
				inc_stack(callee, nextIns, str);
				printf("call : %#x %s\n",callee,str.c_str());
			}
			break;
		case eSUB:
		case eADD:
		case eFRAME:
			{
				char buf[100];
				fin.getline(buf,100);
			}
			break;
		case eRET:
			{
				int nMax = 0;
				UINT ret;
				UINT ins;
				fin >> hex >> ins >> ret >> dec >> nMax;
#if ALL
				dec_stack(ret);
#elif CALL
				stack.pop_back();
#endif
			}
			break;
		}
	}

	print_stack(argv[1]);
#endif

	destory();
	return 0;
}

eTYPE read_type(ifstream& fin)
{
	char cType;
	fin >> cType;

	switch(cType)
	{
	case 'c':
	case 'C':
		return eCALL;
	case 'r':
	case 'R':
		return eRET;
	case 'S':
	case 's':
		return eSUB;
	case 'f':
	case 'F':
		return eFRAME;
	case 'a':
	case 'A':
		return eADD;
	default:
		return eNONE;
	}
}

void read_call(ifstream& fin)
{
	UINT callee, ip, nextIns, caller = 0;
	string str;
	fin >> hex >> ip >> callee >> nextIns;
	fin >> str;

	if(!stack.empty())
		caller = stack.back()->callee;

	insert_data(callee, caller, nextIns);
}

void read_add(ifstream& fin)
{
	DATA* pTopData = search_data(prev_call.callee, prev_call.caller, prev_call.nextIns);
	if(pTopData)
	{
		int nAdd;
		UINT nextins;
		fin >> dec >> nAdd;
		fin >> hex >> showbase >> nextins;

		if(pTopData->nAdd < nAdd)
			pTopData->nAdd = nAdd;
	}
}

void read_sub(ifstream& fin)
{
	if(stack.empty())
		return;

	SDATA* pTopCall = stack.back();
	if(pTopCall)
	{
		DATA* pData = search_data(pTopCall->callee, pTopCall->caller, pTopCall->nextIns);
		if(pData)
		{
			int nSize;
			UINT esp;
			fin >> dec >> nSize;
			fin >> hex >> esp;

			if(pData->nStack< nSize)
			{
				pData->nStack += nSize;
//				pData->nSavedEbp = EBP_SIZE;
			}
// 				pData->nStack = nSize;
		}
	}
}

void read_ret(ifstream& fin)
{
	int nMax;
	UINT ret, ip;
	fin >> hex >> ip >> ret >> dec >>  nMax;

	if(stack.empty())
		return;

	SDATA* pTopCall = stack.back();
	if(pTopCall)
	{
		if(pTopCall->nextIns != ret)
		{
			printf("pTopCall->nextIns : %#x | ret  : %#x\n", pTopCall->nextIns, ret);
			return;
		}

		DATA* pData = search_data(pTopCall->callee, pTopCall->caller, pTopCall->nextIns);
		if(pData)
		{
			if(pData->nMax < nMax)
				pData->nMax = nMax;
		}
	}

	memcpy(&prev_call,stack.back(),sizeof(SDATA));

	SDATA* pTmp = stack.back();
	delete pTmp;
	stack.pop_back();
}

void read_stack(ifstream& fin)
{
	if(stack.empty())
		return;

	SDATA* pTopCall = stack.back();
	if(pTopCall)
	{
		DATA* pData = search_data(pTopCall->callee, pTopCall->caller, pTopCall->nextIns);
		if(pData)
		{
			int nSize;
			fin >> dec >> nSize;
			if(pData->nStack< nSize)
			{
				pData->nStack += nSize;
//				pData->nSavedEbp = EBP_SIZE;
			}
// 				pData->nStack = nSize;
		}
	}
}

void insert_data(UINT callee, UINT caller, UINT nextIns)
{
	DATA* pTmp = search_data(callee, caller, nextIns);
		
	if(stack.empty() || !pTmp)
	{
		DATA* pNew = new DATA;
		memset(pNew,0,sizeof(DATA));

		pNew->caller = caller;
		pNew->callee = callee;
		pNew->nCount = 1;
		pNew->nStack = 0;
		pNew->nextIns = nextIns;
		listdata.push_back(pNew);
	}
	else
	{
		pTmp->nCount++;
//		int nValue = pTmp->nStack + pTmp->nAdd;
	}

	SDATA* pSdata = new SDATA;
	pSdata->caller = caller;
	pSdata->callee = callee;
	pSdata->nextIns= nextIns;
	stack.push_back(pSdata);
}

DATA* search_data(UINT callee, UINT caller, UINT nextIns)
{
	list<DATA*>::iterator iter = listdata.begin();
	while(iter != listdata.end())
	{
		DATA* pTmp = *iter;
		if(pTmp)
		{
			if(pTmp->callee == callee && pTmp->caller == caller && pTmp->nextIns)
				return pTmp;
		}
		++iter;
	}
	return NULL;

#if 0
	list<DATA*>::iterator iter = listdata.end();
	while(iter != listdata.begin())
	{
		DATA* pTmp = *iter;
		if(pTmp)
		{
			if(pTmp->callee == callee && pTmp->caller == caller)
				return pTmp;
		}
		--iter;
	}
	return NULL;
#endif
}

void print_call(char* filename)
{
	char buf[256]={0,};
	sprintf(buf,"%s_graph.dot",filename);
	ofstream fout;
	fout.open(buf);

	if(!fout.is_open())
	{
		cout << buf << " Write file open failed!!" << endl;
		return;
	}

	fout << "digraph callgraph{"<< endl;
	fout << "graph [" << endl;
	fout << "rankdir =\"LR\""<< endl;
	fout << "];" << endl; 

	list<DATA*>::iterator iter = listdata.begin();
	DATA* pData = NULL;
	while(iter != listdata.end())
	{
		pData = *iter;
		if(pData)
		{
			fout << "\t" << "\"" << showbase << hex << pData->caller
				 << "\"" << "->" << "\"" << showbase << hex << pData->callee
				 << "\"" << "[label=\"" << dec << pData->nStack + pData->nAdd << "\"]" << endl;
			++iter;
		}
	}
	fout << "}" << endl;
}

void print_stack(char* filename)
{
	char buf[256]={0,};
#if bit
	sprintf(buf,"%s_bit.out",filename);
#elif LOG
	sprintf(buf,"%s_log.out",filename);
#else
	sprintf(buf,"%s_stack.out",filename);
#endif 
	ofstream fout;
	fout.open(buf);

	for(int nIndex = 0 ; nIndex < vecStack.size() ; ++nIndex)
	{
		int nIsUp = 0;
		int cmp = vecStack[nIndex]->nStack;

		if(nIndex != 0 && vecStack[nIndex]->funcname == ".plt")
			continue;


		if(nIndex == 0)
		{
#if bit
			//fout << dec << vecStack[nIndex] << " " << nIsUp<< endl;
			fout << dec << nIsUp<< endl;
#else
	#if LOG
			//fout << hex << showbase <<  vecStack[nIndex]->sData.callee << " " << vecStack[nIndex]->sData.caller << " " << dec << vecStack[nIndex]->nStack<< endl;
			fout << hex << showbase <<  vecStack[nIndex]->sData.callee << '\t' << dec << vecStack[nIndex]->nStack<< endl;
	#else
			//fout << dec << vecStack[nIndex]->nStack << hex << showbase << vecStack[nIndex]->sData.callee << endl;
//			fout.width(6);
//			fout << dec << vecStack[nIndex]->nStack;
			fout << dec << vecStack[nIndex]->nStack << endl;
/*****
			if(nIsUp == 1)
			{
				if(vecStack[nIndex]->sData.callee == 0)
					fout << '\t' << hex << showbase << vecStack[nIndex]->sData.callee << endl;
				else
					fout << '\t' << hex << showbase << vecStack[nIndex]->sData.callee << '\t' << vecStack[nIndex]->funcname << endl;
			}
			else
			{
				if(vecStack[nIndex]->sData.callee == 0)
					fout << '\t' << hex << showbase << vecStack[nIndex]->sData.callee << endl;
				else
					fout << '\t' << hex << showbase << vecStack[nIndex]->sData.callee << '\t' << vecStack[nIndex]->funcname << endl;
				//fout << '\t' << hex << showbase << vecStack[nIndex]->sData.caller << endl;
			}
*/

			//fout << dec << vecStack[nIndex]->nStack;
	#endif

			fprintf(pFDepth,"%d\n",vecStack[nIndex]->nDepth);
#endif
			continue;
		}
		int before = vecStack[nIndex-1]->nStack;
		int after = vecStack[nIndex]->nStack;

		if(after > before) 			nIsUp = 1;
		else if(after == before)	nIsUp = 0;
		else						nIsUp = -1;

#if bit
		//fout << dec << vecStack[nIndex] << " " << nIsUp << endl;
		fout << dec << nIsUp << endl;
#else

	#if LOG
		fout << hex << showbase <<  vecStack[nIndex]->sData.callee << " " << vecStack[nIndex]->sData.caller << " " << dec << vecStack[nIndex]->nStack<< endl;
	#else
//		fout << dec << vecStack[nIndex]->nStack << " " << hex << showbase << vecStack[nIndex]->sData.callee << endl;
//			fout.width(6);
//			fout << dec << vecStack[nIndex]->nStack;
			fout << dec << vecStack[nIndex]->nStack << endl;


/*****			
			if(nIsUp == 1)
				fout << '\t' << hex << showbase << vecStack[nIndex]->sData.callee << '\t' <<  vecStack[nIndex]->funcname << endl;
			else
#if CALL
				fout << '\t' << hex << showbase << vecStack[nIndex]->sData.callee << '\t' << vecStack[nIndex]->funcname << endl;
//				fout << '\t' << hex << showbase << vecStack[nIndex]->sData.callee << endl;
#elif ALL
				fout << '\t' << hex << showbase << vecStack[nIndex]->sData.caller << '\t' << vecStack[nIndex]->funcname << endl;
//				fout << '\t' << hex << showbase << vecStack[nIndex]->sData.caller << endl;
#endif
	//	fout << dec << vecStack[nIndex]->nStack;
******/
	#endif
		fprintf(pFDepth,"%d\n",vecStack[nIndex]->nDepth);

	//	if(nIndex%3 == 2)
	//		fout << endl;

#endif
	}

#if 0
	fout << "\n\n" << endl;
	fout << "MAX USAGE STACK" << endl;
	list<DATA*>::iterator d_iter = listdata.begin();
	DATA* pData = NULL;
	while(d_iter != listdata.end())
	{
		pData = *d_iter;
		if(pData)
			fout << dec << pData->nStack + pData->nAdd << " " << dec << pData->nMax << endl;
		++d_iter;
	}
#endif
}

void destory()
{
	list<DATA*>::iterator iter = listdata.begin();
	DATA* pData = NULL;
	while(iter != listdata.end())
	{
		pData = *iter;
		if(pData)
			delete pData;
		++iter;
	}

	listdata.clear();
}

void inc_stack(UINT callee, UINT nextIns, string& str)
{
	UINT caller = 0;
	if(!stack.empty())
		caller = (stack.back())->callee;

	DATA* pData = search_data(callee, caller, nextIns);
	if(pData)
	{
		int nValue = 0;
		int nSize = vecStack.size();
		if(!vecStack.empty())
		{
			SINFO* pInfo = vecStack.back();
			if(pInfo)
				nValue = pInfo->nStack;
		}

		//nValue += (pData->nStack + pData->nAdd);
		//nValue += (pData->nStack + pData->nAdd + pData->nSavedEbp);
#if ALL
		nValue += (pData->nStack + pData->nAdd + pData->nSavedEbp);
#elif CALL
		nValue = (pData->nStack + pData->nAdd + pData->nSavedEbp);
#endif

		SINFO* pNew = new SINFO;
		pNew->sData.caller = caller;
		pNew->sData.callee = callee;
		pNew->nStack = nValue + RET_SIZE;
//		pNew->nStack = nValue;
		pNew->nDepth = ++nDepth;
		pNew->funcname = str;

		vecStack.push_back(pNew);
	//	printf("C %#x : %d\n", callee, nValue);
	}
	else
	{
		printf("[72 : ]Not Fount Data\n");
	}

	SDATA* pSdata = new SDATA;
	pSdata->caller = caller;
	pSdata->callee = callee;	
	pSdata->nextIns = nextIns;
	stack.push_back(pSdata);
}

void dec_stack(UINT ret)
{
	if(stack.empty())
		return;

	SDATA* pTopCall = stack.back();
	if(pTopCall)
	{
		if(pTopCall->nextIns != ret)
		{
			printf("pTopCall->nextIns : %#x | ret : %#x\n", pTopCall->nextIns, ret);
			return;
		}

		DATA* pData = search_data(pTopCall->callee, pTopCall->caller, pTopCall->nextIns);
		if(pData)
		{
			printf("ret  : %#x\n", pTopCall->caller);
			int nValue = pData->nStack + pData->nAdd + pData->nSavedEbp;
		
			nValue = vecStack.back()->nStack - nValue;

			SINFO* pNew = new SINFO;
			pNew->sData.caller = pTopCall->caller;
			pNew->sData.callee = pTopCall->callee;
			pNew->nStack = nValue;
			pNew->nDepth = --nDepth;

			vecStack.push_back(pNew);
			stack.pop_back();
			//		printf("R %#x : %d\n", nMax, nValue);
		}
	}
}
