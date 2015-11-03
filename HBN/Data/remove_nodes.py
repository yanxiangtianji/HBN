import os,sys,string,re

#NODE_FILE_NAME='node.txt'
NODE_FILE_NAME='node_traffic.txt'
#KNOWLEDGE_FILE_NAME='knowledge.txt'
KNOWLEDGE_FILE_NAME='knowledge_traffic.txt'

def get_working_folder():
    folder='';
    if len(sys.argv)==2:
        folder=sys.argv[1];
    else:
        folder=input('Please into the folder you want to do pre-process: ');
    return str(folder);    

def cut_node_name_and_return(fn_node,node_fomrat_to_cut):
    #backup the original data.
    if os.path.exists(fn_node+'.bak'):
        print fn_node,'had been backuped. Use the backuped one!';
    else:
        print 'rename:',fn_node;
        os.rename(fn_node,fn_node+'.bak');
    #generate new data
    f=open(fn_node+'.bak','r');
    content=f.read().split('\n');
    f.close();
    num_nodes=int(content[0]);
    node_name_to_cut=set();
    output=[];
    for line in content[1:]:
        if len(line)<3:
            continue;
        pair=line.split(' ');
        if re.match(node_fomrat_to_cut,pair[0]):
            node_name_to_cut.add(pair[0])
        else:
            output.append(line);
    f=open(fn_node,'w');
    f.write(str(num_nodes-len(node_name_to_cut))+'\n');
    f.write(string.join(output,'\n'));
    f.close();
    return node_name_to_cut


def work_knowledge(fn_knowledge,node_name_to_cut,force_redo):
    #backup the original data.
    if os.path.exists(fn_knowledge+'.bak'):
        print fn_knowledge,'had been backuped.';
        if force_redo!=True:
            print 'Skip it!'
            return;
        else:
            print 'Use the backuped one!'
    else:
        print 'rename:',fn_knowledge;
        os.rename(fn_knowledge,fn_knowledge+'.bak');
    #generate new data
    f=open(fn_knowledge+'.bak','r');
    content=f.read().split('\n');
    f.close();
        #calculate new layer
    output=[];
    num_per_layer=[];
    max_layer=-1;
    for line in content:
        if len(line)<3:
            continue;
        pair=line.split(' ');
        if pair[0] not in node_name_to_cut:
            output.append(pair)
            max_layer=max(max_layer,int(pair[1]));
            while len(num_per_layer)<=max_layer:
                num_per_layer.append(0);
            num_per_layer[int(pair[1])]+=1;
#    print max_layer
#    print num_per_layer
#    print output
    new_layer_map=range(0,len(num_per_layer))
    current_layer=-1;
    for l in range(0,len(num_per_layer)):
        if num_per_layer[l]!=0:
            current_layer+=1
        new_layer_map[l]=current_layer
#    print new_layer_map;
    #write new data
    f=open(fn_knowledge,'w');
    for pair in output:
        f.write(pair[0])
        f.write(' ')
        f.write(str(new_layer_map[int(pair[1])]))
        f.write('\n')
    f.close();

def work_future_conf(fn_future,force_redo):
    #backup the original data.
    if os.path.exists(fn_future+'.bak'):
        print fn_future,'had been backuped.';
        if force_redo!=True:
            print 'Skip it!'
            return;
        else:
            print 'Use the backuped one!'
    else:
        print 'rename:',fn_future;
        os.rename(fn_future,fn_future+'.bak');
    #generate new data
    fin=open(fn_future+'.bak','r');
    fout=open(fn_future,'w');
    content=fin.readline()
    fout.write(content) #copy first line
    content=fin.readline()  #ignore seconde line
    fout.write('1') #directly write '1'
    fin.close();
    fout.close();

'''
add one to each state, and make miss state to be 0
'''
def work_data(fn_data,need_process,force_redo):
    if os.path.exists(fn_data+'.bak'):
        print fn_data,'had been backuped.';
        if force_redo!=True:
            print 'Skip it!'
            return;
        else:
            print 'Use the backuped one!'
    else:
        print 'rename:',fn_data;
        os.rename(fn_data,fn_data+'.bak');
    fin=open(fn_data+'.bak','r');
    fout=open(fn_data,'w');
    #head
    head=fin.readline();    # a '\n' will be read in
    head=head[:-1].split(',');
    to_remain=[]
    for i in range(0,len(head)):
        if head[i] not in need_process:
            to_remain.append(i);
    to_remain.sort();
    output=[]
#    print to_remain
    for i in range(0,len(to_remain)):
#        print to_remain[i], head[to_remain[i]]
        output.append(head[to_remain[i]]);
    fout.write(string.join(output,','))
    fout.write('\n')
    #content
    count=0;
    cache_range=range(0,len(to_remain));
    for line in fin:
        if len(line)<3:
            continue;
        items=line[:-1].split(',');
        output=[];
        for i in cache_range:
            output.append(items[to_remain[i]]);
        nline=string.join(output,',');
        fout.write(nline);
        fout.write('\n');
        count+=1;
        if count%10000==0:
            print count,'piece of data had been processed.';
    fin.close();
    fout.close();

def work(folder,node_fomrat_to_cut,process_future,force_redo):
    print 'Getting nodes to process...'
    need_process=cut_node_name_and_return(folder+'/'+NODE_FILE_NAME,node_fomrat_to_cut);
    print 'Nodes need to be process:',need_process;
    work_knowledge(folder+'/'+KNOWLEDGE_FILE_NAME,need_process,force_redo);
#    work_knowledge(folder+'/knowledge.txt',need_process,False);
    if process_future=True:
        print 'Processing future conf...'
        work_future_conf(folder+'/future.conf',force_redo);
    print 'Processing train file...';
    work_data(folder+'/train.data.csv',need_process,force_redo);
#    work_data(folder+'/train.data.csv',need_process,False);
    print 'Processing test file...';
    work_data(folder+'/test.data.csv',need_process,force_redo);
#    work_data(folder+'/test.data.csv',need_process,True);

def restore_file(filename):
    if os.path.exists(filename+'.bak'):
        os.rename(filename+'.bak',filename);
        return True
    else:
        return False
    
def restore(folder,process_future):
    restore_file(folder+'/'+NODE_FILE_NAME);
    restore_file(folder+'/'+KNOWLEDGE_FILE_NAME);
    if process_future==True:
        restore_file(folder+'/fucture.conf')
    restore_file(folder+'/train.data.csv');
    restore_file(folder+'/test.data.csv');

if __name__ == '__main__':
#    print sys.argv
    print 'start'
    folder=get_working_folder()
    node_fomrat_to_cut='''(current_)|(dynamic_past_).+''';
    force_redo=True;
    process_future=True;
    if os.path.exists(folder):
        work(folder,node_fomrat_to_cut,process_future,force_redo);
        #restore(folder,process_future);
    else:
        print 'wrong folder!';
    print 'end'
