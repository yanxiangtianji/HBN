import os

def modify_table(name):
    modified=False
    f=open(name,'r');
    raw=f.read();
    content=raw.split('\n');
    f.close();
    buf=''
    for line in content:
        if line=='Real_m	0	0		0':
            modified=True;
        else:
            buf+=(line+'\n')
    if modified:
        print 'Modified:',name
        f=open(name,'w')
        f.write(buf)
        f.close()
        

def modify_line(name):
    pass

folders=range(48,53+1)
#folders=[48]
for folder in folders:
    folder=str(folder)
    l=os.listdir(folder)
    for name in l:
        if name[0]=='p' and os.path.isdir(folder+'/'+name):
            path=folder+'/'+name+'/'
            l2=os.listdir(path)
            for f in l2:
                if f=='test_result_table.txt' or f=='test1_result_table.txt':
                    modify_table(path+f);
#                elif f=='test_result_line.txt' or f=='test1_result_line.txt':
#                    modify_line(path+f);
