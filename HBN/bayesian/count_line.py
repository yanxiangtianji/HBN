import glob,time

count=0;
filelist=glob.glob('*.cpp')+glob.glob('*.h');
for fname in filelist:
    f=open(fname);
    local_count=0
    for line in f:
        local_count=local_count+1;
    print fname,local_count
    count=count+local_count

print '\n',count,'lines in all.';
#print 'Press any key to continue'
input('Press any key to continue')
#time.sleep(4)
