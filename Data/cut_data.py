import os,string

def get_settings():
    f=open('cut_data_setting.txt')
    a=f.read();
    f.close();
    b=a.split('\n');
    while b.count('')!=0:
        b.remove('');
    if len(b)!=3:
        return;
    b[1]=b[1].split(' ');   #test sub-folders
    b[2]=b[2].split(' ');   #nodes to remove
    while b[1].count('')!=0: b[1].remove('');
    while b[2].count('')!=0: b[2].remove('');
    return (b[0],b[1],b[2]);

def modify_node(fn_node,rm_column):
    if not os.path.exists(fn_node):
        print 'Cannot open node file',fn_node;
        exit(1)
    #generate new data
    f=open(fn_node,'r');
    content=f.read().split('\n');
    f.close();
    num=0
    buf=''
    for line in content[1:]:
        if len(line)<3:
            continue;
        pair=line.split(' ');
        if pair[0] not in rm_column:
            buf+=(line+'\n');
            num+=1
    f=open(fn_node,'w');
    f.write(str(num))
    f.write('\n')
    f.write(buf);
    f.close();

def modify_knowledge(fn_knowledge,rm_column):
    if not os.path.exists(fn_knowledge):
        print 'Cannot open knowledge file',fn_node;
        exit(1)
    #generate new data
    f=open(fn_knowledge,'r');
    content=f.read().split('\n');
    f.close();
    f=open(fn_knowledge,'w');
    for line in content:
        if len(line)<3:
            continue;
        pair=line.split(' ');
        if pair[0] not in rm_column:
            f.write(line+'\n');
    f.close();

def modify_data(fn_data,rm_column):
    if os.path.exists(fn_data+'.bak'):
        print fn_data,'had been backuped.';
        print 'adopting the backuped file.'
    else:
        print 'rename:',fn_data;
        os.rename(fn_data,fn_data+'.bak');
    fin=open(fn_data+'.bak','r');
    fout=open(fn_data,'w');
    #head
    head=fin.readline()[:-1].split(',');    # a '\n' will be read in
    buff='';
    offset=[]
    n=len(head)
    for name in head:
        if name in rm_column:
            offset.append(False)
        else:
            offset.append(True)
            buff+=name+','
    fout.write(buff[:-1])
    fout.write('\n');
    #content
    count=0;
    for line in fin:
        if len(line)<3:
            continue;
        count+=1;
        items=line[:-1].split(',');
        new_items=[]
        for i in range(n):
            if offset[i]:
                new_items.append(items[i])
        new_line=string.join(new_items,',');
        fout.write(new_line);
        fout.write('\n');
        if count%10000==0:
            print count,'piece of data had been processed.';
    fin.close();
    fout.close();

def modify_conf(folder,rm_column):
    print 'Processing nodes.'
    modify_node(folder+'/node_traffic.txt',rm_column);
    print 'Processing knowledge.'
    modify_knowledge(folder+'/knowledge_traffic.txt',rm_column);

def main():
    folder,sub_folders,rm_column=get_settings()
    print folder,'\n',rm_column
    modify_conf(folder,rm_column)
    print 'Processing data.'
    print '\ttrain data:'
    modify_data(folder+'/data.csv',rm_column);
    for sub in sub_folders:
        print '\ttest data (',sub,'):'
        modify_data(folder+'/'+sub+'/data.csv',rm_column);


if __name__=='__main__':
    main();
