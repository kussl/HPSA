#!/usr/local/bin/python

import csv,os,sys,datetime
from os import listdir
from os.path import isfile, join
import statistics as stats 

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
	obj = [x for x in content[-1].split(' ') if (x.replace('.','').replace('E-','').strip()).isnumeric() ]
	obj = obj[-1]
	return round(float(obj), 8)


def recallres(size):
	ts = datetime.datetime.now() 
	path = "baron/"
	fname = path+str(ts)+".csv"
	f = open(fname,"w")
	writer = csv.writer(f) 
	data = [] 
	filenames = os.listdir(path) 
	filenames = [f.split('.')[0] for f in filenames if f.find('.tim') > -1]
	for i in range(len(filenames)): 
		rec = [] 
		t = readtim(path,filenames[i])
		if not t: 
			os.remove(fname)
			return None 
		r = readres(path,filenames[i])
		rec.append([i,t,r])
		data.append([i,t,r])
		writer.writerow(rec) 

	f.close()
	print("Time: ", end=" ")
	maxtime = max(data, key=lambda x: x[1])[1]
	mintime = min(data, key=lambda x: x[1])[1]
	avgtime = (maxtime+mintime)/2
	maxobj = max(data, key=lambda x:x[2])[2]
	minobj = min(data, key=lambda x:x[2])[2]
	avgobj = (maxobj+minobj)/2
	print("Max:", maxtime, "Min:", mintime, "Avg:", avgtime)
	print("Obj: ", end=" ")
	print("Max:", maxobj, "Min:",minobj, "Avg:",avgobj)

	#Now purge the files
	files = [f for f in listdir(path) if f.find(".tim") > -1 or f.find(".res") > -1 or f.find(".opt") > -1 or f.find("program_") > -1 ]
	for f in files: 
		os.remove(path+f)

print("Collecting results..")

size = int(sys.argv[1])
print('Size: ', size) 

recallres(size)
