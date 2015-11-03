import os,sys,string

basic_need_pro=['current_0','current_1','current_2','current_3',
    'dynamic_past_0','dynamic_past_1','dynamic_past_2','dynamic_past_3'];

def get_working_folder():
    folder='';
    if len(sys.argv)==2:
        folder=sys.argv[1];
    else:
        folder=input('Please into the folder you want to do pre-process: ');
    return str(folder);    

def load_knowledge(fn_knowledge):
    f=open(fn_knowledge);
    content=f.read().split('\n');
    f.close();
    res=[];
    for line in content:
        if len(line)<3:
            continue;
        pair=line.split(' ');
        if pair[1]!='0':
            res.append(pair[0]);
    return res;

def work_node(fn_node,need_process):
    #backup the original data.
    if os.path.exists(fn_node+'.bak'):
        print fn_node,'had been backuped. Skip it!';
        return;
    else:
        print 'rename:',fn_node;
        os.rename(fn_node,fn_node+'.bak');
    #generate new data
    f=open(fn_node+'.bak','r');
    content=f.read().split('\n');
    f.close();
    f=open(fn_node,'w');
    f.write(content[0]+'\n');
    for line in content[1:]:
        if len(line)<3:
            continue;
        pair=line.split(' ');
        if pair[0] in need_process:
            f.write(pair[0]+' '+str(int(pair[1])+1)+'\n');
        else:
            f.write(line+'\n');
    f.close();

'''
add one to each state, and make miss state to be 0
'''
def work_data(fn_data,need_process):
    if os.path.exists(fn_data+'.bak'):
        print fn_data,'had been backuped. Skip it!';
        return;
    else:
        print 'rename:',fn_data;
        os.rename(fn_data,fn_data+'.bak');
    fin=open(fn_data+'.bak','r');
    fout=open(fn_data,'w');
    #head
    head=fin.readline();    # a '\n' will be read in
    fout.write(head);
    head=head[:-1].split(',');
    offset=[]
    for name in need_process:
        offset.append(head.index(name));
    offset.sort();
    #content
    count=0;
    for line in fin:
        if len(line)<3:
            continue;
        items=line.split(',');
        for off in offset:
            items[off]=str(1+int(items[off]));
        nline=string.join(items,',');
        fout.write(nline);
        fout.write('\n');
        count=count+1;
        if count%10000==0:
            print count,'piece of data had been processed.';
    fin.close();
    fout.close();

def work(folder,test_folder_list):
    need_process=basic_need_pro+load_knowledge(folder+'/knowledge_traffic.txt');
    need_process=set(need_process);
    print 'Nodes need to be process:',need_process;
    work_node(folder+'/node_traffic.txt',need_process);
    print 'Processing train file';
    work_data(folder+'/data.csv',need_process);
    for test_folder in test_folder_list:
        if os.path.isdir(folder+'/'+test_folder):
            print 'Processing test file:',test_folder;
            work_data(folder+'/'+test_folder+'/data.csv',need_process);
        else:
            print 'Test folder do not exist:',test_folder;
    
if __name__ == '__main__':
#    print sys.argv
    print 'start'
    folder=get_working_folder()
    test_folder_list=['test','test1','test2'];
    if os.path.exists(folder):
        work(folder,test_folder_list);
    else:
        print 'wrong folder!';
    print 'end'
