#!/usr/local/bin/python3.7

import csv,os,sys,datetime
from os import listdir
from os.path import isfile, join

def readtim(path,i):
	try: 
		f = open(path+"/"+str(i)+".tim","r")
	except Exception as s: 
		return None 
	content = [s for s in f]
	ts = [x for x in content[-1].split(' ') if x.replace('.','').isnumeric() ][-1]
	return float(ts) 

def readres(path,i):
	f = open(path+"/"+str(i)+".res","r")
	content = [s for s in f]
	content = [x for x in content[-1].split(' ') if (x.replace('.','').replace('E-','').strip()).isnumeric() ]

	obj = content[-1]
	return float(obj) 


def recallres(size):
	ts = datetime.datetime.now() 
	path = "baron/"
	fname = path+str(ts)+".csv"
	f = open(fname,"w")
	writer = csv.writer(f) 
	maxobj = 0 
	maxtime = 0 
	for i in range(size): 
		rec = [] 
		t = readtim(path,i)
		if not t: 
			os.remove(fname)
			return None 
		r = readres(path,i)
		if maxobj < r: 
			maxobj = r 
		if maxtime < t: 
			maxtime = t
		rec.append([i,t,r])
		writer.writerow(rec) 

	f.close()

	print("Max time: ", maxtime, "and max objective: ", maxobj)

	#Now purge the files
	files = [f for f in listdir(path) if f.find(".tim") > -1 or f.find(".res") > -1 or f.find(".opt") > -1 or f.find("program_") > -1 ]
	for f in files: 
		os.remove(path+f)

print("Collecting results..")

size = int(sys.argv[1])
print('Size: ', size) 

recallres(size-1)
