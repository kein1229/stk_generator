#!/usr/bin/env python
	
import os
import sys
import code
import pprint
import sqlite3
import StringIO
import tempfile
import time
import numpy as np
from os import path

from binascii import unhexlify

try:
	import psyco
	psyco.log()
	psyco.full()
except ImportError:
	pass


class DynamicLCS(object):
	def __init__(self,setA,setB):
		self.times   = []
		self.files 	 = []
		self.sameSet = []
		self.setA 	 = setA
		self.setB 	 = setB

	def lcs(self, s1,s2):
		lengths = [[0 for j in range(len(s2)+1)] for i in range(len(s1)+1)]

		# row 0 and column 0 are initialized to 0 already
		for i, x in enumerate(s1):
			for j, y in enumerate(s2):
				if x == y:
					lengths[i+1][j+1] = lengths[i][j] + 1
				else:
					lengths[i+1][j+1] = \
						max(lengths[i+1][j], lengths[i][j+1])

		# read the substring out from the matrix
		result = []
		x, y = len(s1), len(s2)
		while x != 0 and y != 0:
			if lengths[x][y] == lengths[x-1][y]:
				x -= 1
			elif lengths[x][y] == lengths[x][y-1]:
				y -= 1
			else:
				assert s1[x-1] == s2[y-1]
#				result = s1[x-1] + result
				result.insert(0,s1[x-1])
#				result.reverse()
				x -= 1
				y -= 1
		return result


	def similarity(self):
		instSizeAll = len(self.setA)
		sameSet     = len(self.sameSet)
		
		similarity  = (float(sameSet)/instSizeAll)*100.0
		print similarity
		
		

def main(argvs):
	fileA = argvs[1]
	fileB = argvs[2]

	t = time.time()
	f0 = open(fileA,'r')
	f1 = open(fileB,'r')

	setA = f0.readlines()
	setB = f1.readlines()

	for cnt0, set0 in enumerate(setA):
		setA[cnt0] = set0.strip('\n')

	for cnt0, set0 in enumerate(setB):
		setB[cnt0] = set0.strip('\n')
	
	prism = DynamicLCS(setA,setB)
	prism.times.append(["LCS File read 1:", (time.time()-t)])
	t = time.time()
#	print lcsPrism.lcs(['111','222','333','33','444','555','666'],\
#					   ['111','333','33','444','555','666'])
	prism.sameSet = prism.lcs(setA,setB)
	prism.similarity()
	prism.times.append(["Elapsed Time :", (time.time()-t)])

	ntime = prism.times
	print ntime[0][0],'\t',ntime[1][0],'\t'
	print ntime[0][1],'\t',ntime[1][1],'\t'
	
	print "LCS process end"

def usage():
	print "Usage:", sys.argv[0], "<prog 1> <prog 2>"


if __name__ == "__main__":
	if len(sys.argv) != 3:
		usage()
	else:
		main(sys.argv)
