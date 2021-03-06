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
	if obj:
		obj = obj[-1]
		return round(float(obj), 8)
	else:
		return 0.0

def recallres(size,ex_time,silent):
	ts = datetime.datetime.now() 
	path = "baron/"
	# fname = path+str(ts)+".csv"
	# f = open(fname,"w")
	# writer = csv.writer(f) 
	data = [] 
	filenames = os.listdir(path) 
	filenames = [f.split('.')[0] for f in filenames if f.find('.tim') > -1]

	for i in range(len(filenames)): 
		rec = [] 
		t = readtim(path,filenames[i])
		if t is None: 
			os.remove(fname)
			return None 
		r = readres(path,filenames[i])
		rec.append([i,t,r])
		data.append([i,t,r])

		#writer.writerow(rec) 

	#f.close()
	if silent !=1:
		print("Time: ", end=" ")
	t = [x[1] for x in data]
	maxtime = round(max(t)+ex_time,3) #max(data, key=lambda x: x[1])[1]
	mintime = round(min(t)+ex_time,3) #min(data, key=lambda x: x[1])[1]
	avgtime = round(stats.mean(t)+ex_time,3) #(maxtime+mintime)/2
	totaltime = sum(t)
	maxobj = max(data, key=lambda x:x[2])[2]
	minobj = min(data, key=lambda x:x[2])[2]
	avgobj = (maxobj+minobj)/2
	if silent != 1:
		print("Max:", maxtime, "Min:", mintime, "Avg:", avgtime, "Total:", totaltime)
		print("Obj: ", end=" ")
		print("Max:", maxobj, "Min:",minobj, "Avg:",avgobj)

	#Now purge the files
	files = [f for f in listdir(path) if f.find(".tim") > -1 or f.find(".res") > -1 or f.find(".opt") > -1 or f.find("program_") > -1 ]
	for f in files: 
		os.remove(path+f)

size = int(sys.argv[1])
ex_time = float(sys.argv[2])
silent = int(sys.argv[3])


#print('Size: ', size) 

recallres(size,ex_time,silent)
#print("------------------------------")