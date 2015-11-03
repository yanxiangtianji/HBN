'''
Created on 2013-08-25

@author: Tian Zhou
'''
import os,sys,subprocess;

margin='---------';
CONF_FILE_NAME='go_folder.txt'

def get_folder():
    if len(sys.argv)>1:
        return (sys.argv[1],sys.argv[2]);
    f=open(CONF_FILE_NAME,'r');
    a=f.read();
    f.close();
    b=a.split('\n');
    while b.count('')!=0:
        b.remove('');
    if len(b)!=3:
        return;
    b[1]=b[1].split(' ');
    b[2]=b[2].split(' ');
    while b[1].count('')!=0: b[1].remove('');
    while b[2].count('')!=0: b[2].remove('');
    return (b[0],b[1],b[2]);

def get_models():
    if len(sys.argv)>3:
        return sys.argv[3:];
    return [200,500,1000,1500,2000,3000,5000];

def write_learn_param(filename,improvement,iteration):
    f=open(filename,'w');
    f.write(str(improvement)+' '+str(iteration)+'\n');
    f.write('0 1.0\n0 1.0\n');
    f.close();

def write_current_folder(folder,model,testdata=None):
    line=folder+'\n'+model+'\n';
    if testdata!=None:
        line=line+testdata+'\n';
    f=open('./current_folder.txt','w');
    f.write(line);
    f.close();
    
def train(folder,model):
    print folder,model;
    print margin*2+'----Train'+margin*2;
    write_current_folder(folder,model);
    oldcwd=os.getcwd();
    os.chdir('../Release');
    ptr=subprocess.Popen('./train.exe');
    ptr.wait();
    os.chdir(oldcwd);
    print margin*2+'End Train'+margin*2;

def generate(fodler,model):
    print folder;
    print margin*2+'----Generate'+margin*2;
    write_current_folder(folder,model);
    oldcwd=os.getcwd();
    os.chdir('../Release');
    ptr=subprocess.Popen('./generate.exe');
    ptr.wait();
    os.chdir(oldcwd);
    print margin*2+'End Generate'+margin*2;

def test(folder,model,testdata):
    print folder,model,testdata;
    print margin+'----Test'+margin;
    write_current_folder(folder,model,testdata);
    oldcwd=os.getcwd();
    os.chdir('../Release');
    ptr=subprocess.Popen('./test.exe');
#    ptr=subprocess.Popen('./test_normal.exe');
#    ptr=subprocess.Popen('./test_post.exe');
    ptr.wait();
    os.chdir(oldcwd);
    print margin+'End Test'+margin;
    
def enum_works(folder,improvements,testdatas,isLearn=True):
    '''
    folder: base dir of all parameters;
    improvements: list of integers, indicating min_improvement
    testdatas: list of sub-folders of test data file
    '''
    for m in improvements:
        model='p'+str(m);
        if os.path.exists('./'+folder+'/'+model)==False:
            os.mkdir('./'+folder+'/'+model);
            write_learn_param('./'+folder+'/'+model+'/learn_param.txt',m,100);
        #Train or generate:
        if isLearn==False:
            pass;
        elif m.isdigit():
            train(folder,model);
        else:
            generate(folder,model);
#        continue;  #skip testing
        #Test:
        for td in testdatas:
            test(folder,model,td);

def main():
#    print sys.argv
    print 'start'
    folder,improvements,testdatas=get_folder();
    if folder=='' or improvements==[] or testdatas==[]:
        print 'Error: cannot get destination folder!';
        exit;
    print 'Folder:',folder,'\nImprovement:',improvements,'\nTest-data:',testdatas;
    doLearning=True;
#    doLearning=False;
    enum_works(folder,improvements,testdatas,doLearning);
    print 'end'

def main_for_several(folders):
    print folders;
    f=open(CONF_FILE_NAME,'r')
    content=f.read().split('\n')
    f.close();
    for folder in folders:
        f=open(CONF_FILE_NAME,'w')
        f.write(str(folder))
        f.write('\n')
        f.write(content[1])
        f.write('\n')
        f.write(content[2])
        f.write('\n')
        f.close()
        print folder
        main()

if __name__ == '__main__':
    main()
#    main_for_several(range(48,53+1))


