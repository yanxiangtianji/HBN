package hbn;

import java.util.ArrayList;
import java.util.Comparator;

public class Node implements Comparator<Node>{
	private String name;
	private int layer;
	private int nState;
	private ArrayList<Node> parents=new ArrayList<Node>();
	private float[][] CDT;
	private float[] defaultMD;
	private float[] cacheMD;
	
	//constructor
	Node(String name,int nState){
		this(name,nState,0);
	}
	Node(String name,int nState,int layer){
		this.name=name;
		this.nState=nState;
		this.layer=layer;
	}
	
	//parents
	public Node getParent(int idx){
		return parents.get(idx);
	}
	public void addParent(Node n){
		parents.add(n);
	}
	
	//cdt encode/decode
	public int encodeParent(int values[]){
		int res=0;
		for(int i=0;i<parents.size();i++){
			res=res*parents.get(i).nState+values[i];
		}
		return res;
	}
	public int[] decodeParents(int code){
		int[] res=new int[parents.size()];
		for(int i=parents.size()-1;i>=0;i--){
			int temp=parents.get(i).nState;
			res[i]=code%temp;
			code/=temp;
		}
		return res;
	}
	
	//cdt
	public void initCDT(){
		int num=(parents.size()==0?0:1);
		for(Node n: parents){
			num*=n.getnState();
		}
//		CDT=new float[num][nState];
		CDT=new float[num][];
	}
	public int getnCDT(){
		return CDT.length;
	}
	public void setCDT(int offset, float[] cd) throws Exception{
		if(cd.length==nState)
			CDT[offset]=cd;
		else{
			throw new Exception("Length of argmuent("+cd.length+
					") do not fit number of state of this node("+nState+")");
		}
	}
	public void setCDT(int[] parentsValues, float[] cd) throws Exception{
		if(parentsValues.length==parents.size()){
			setCDT(encodeParent(parentsValues),cd);
		}else{
			throw new Exception("Length of parent values("+parentsValues.length+
					") do not fit number of parent nodes("+parents.size()+")");
		}
	}
	public float[] getCDT(int offset){
		if(CDT[offset]!=null)
			return CDT[offset];
		return defaultMD;
//		return CDT[offset]=defaultMD;
	}
	public float[] getCDT(int[] parentsValues) throws Exception{
		if(parentsValues.length!=parents.size()){
			throw new Exception("Length of parent values("+parentsValues.length+
					") do not fit number of parent nodes("+parents.size()+")");
		}
		int code=encodeParent(parentsValues);
		return getCDT(code);
	}
	
	//md
	public void setCacheMD(int state){
		float[] r=new float[nState];
		r[state]=1.0f;
		setCacheMD(r);
	}
	public float[] getCacheMD(){
		if(cacheMD==null)
			try {
				cacheMD=calMD();
			} catch (Exception e) {
				e.printStackTrace();
			}
		return cacheMD;
	}
	private float[] calMD() throws Exception{
		float[][] parMD=new float[parents.size()][];	//in real parent order
		int[] parConsiderOrder=new int[parents.size()];	//in the order of calculation convenience
		int[] numOfZero=new int[parents.size()];
		for(int i=0;i<parents.size();i++){
			parMD[i]=parents.get(i).getCacheMD();
			parConsiderOrder[i]=i;
			for(float f : parMD[i])
				if(f==0.0)
					++numOfZero[i];
		}
		//To speed up the calculation, sort the parents by the number of 0.0 in their marginal distribution from more to less.
		sort2By1(numOfZero,parConsiderOrder);
		numOfZero=null;
		//P(R=Rx,A=A1,B=B1,C=C1) = P(R=Rx|A=A1,B=B1,C=C1)*P(A=A1)*P(B=B1)*P(C=C1)
		//P(R=Rx) = SUM{ P(R=Rx|cond)*P(cond) } for all cond
		float[] res=new float[nState];
		int[] cond_temp=new int[parents.size()];	//in real parent order
		_dfs_accumulate_MD(res,parMD,parConsiderOrder,0,1.0f,cond_temp);
		return res;
	}
	private void _dfs_accumulate_MD(float[] res, float[][] parMD, int[] parConsiderOrder,
			int pos, float pcond, int[] cond) throws Exception{
		if(pcond==0.0){
			return;
		}else if(pos>=parents.size()){
			float[] cd=getCDT(cond);
			for(int i=0;i<res.length;i++)
				res[i]+=pcond*cd[i];
			return;
		}
		int realParentNum=parConsiderOrder[pos];
		int cur_par_nState=parents.get(realParentNum).getnState();
		float[] cur_par_MD=parMD[realParentNum];
		for(int st=0;st<cur_par_nState;++st){
			if(cur_par_MD[st]!=0.0){
				cond[realParentNum]=st;
				_dfs_accumulate_MD(res,parMD,parConsiderOrder,pos+1,pcond*cur_par_MD[st],cond);
			}
		}
	}
	//sort 3 arrays by the first one (from big to small)
	private static void sort2By1(int[] bench, int[] load1){
		for(int i=1;i<bench.length;++i){
			int key=bench[i];
			int t1=load1[i];
			int j=i-1;
			for(;j>=0 && bench[j]<key;--j){
				bench[j+1]=bench[j];	load1[j+1]=load1[j];
			}
			bench[j+1]=key;	load1[j+1]=t1;
		}
	}
	
//	public static void main(String[] args){
//		int len=10;
//		int[] bench=new int[len];
//		int[] load1=new int[len];
//		float[][] load2=new float[len][];
//		Random r=new Random();
//		for(int i=0;i<len;i++){
//			bench[i]=10+r.nextInt(90);
//			load1[i]=i;
//		}
//		for(int i:bench)
//			System.out.print(i+" ");
//		System.out.print("\n");
//		for(int i:load1)
//			System.out.print(" "+i+" ");
//		System.out.print("\n");
//		sort3By1(bench,load1,load2);
//		for(int i:bench)
//			System.out.print(i+" ");
//		System.out.print("\n");
//		for(int i:load1)
//			System.out.print(" "+i+" ");
//	}
	
	//state judgment
	public int getStateFromMD(){
		return tellStateFromDistribution(getCacheMD());
	}
	public static int tellStateFromDistribution(float[] dis){
		if(dis==null){
			return -1;
		}
		int r=-1;
		float max=-Float.MAX_VALUE;
		for(int i=0;i<dis.length;i++){
			if(dis[i]>max){
				max=dis[i];
				r=i;
			}
		}
		return r;
	}
	

	@Override
	public int hashCode() {
		return name.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (!(obj instanceof Node))
			return false;
		Node other = (Node) obj;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		return true;
	}
	
	//compareTo:	
	@Override
	public int compare(Node o1, Node o2) {
		return o1.name.compareTo(o2.name);
	}
	
	//Trivial Getters & Setters:
	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public int getLayer() {
		return layer;
	}

	public void setLayer(int layer) {
		this.layer = layer;
	}

	public int getnState() {
		return nState;
	}

	public void setnState(int nState) {
		this.nState = nState;
	}

	public ArrayList<Node> getParents() {
		return parents;
	}

	public void setParents(ArrayList<Node> parents) {
		this.parents = parents;
	}
	
	public float[][] getCDT() {
		return CDT;
	}
	public void setCDT(float[][] cDT) {
		CDT = cDT;
	}
	public float[] getDefaultMD() {
		return defaultMD;
	}
	public void setDefaultMD(float[] defaultMD) {
		this.defaultMD = defaultMD;
	}
	public void setCacheMD(float[] cacheMD) {
		this.cacheMD = cacheMD;
	}

}
