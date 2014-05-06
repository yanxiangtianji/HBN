package hbn;

import java.util.ArrayList;
import java.util.Comparator;

public class Node implements Comparator<Node>{
	private String name;
	private int layer;
	private int nState;
	private ArrayList<Node> parents=new ArrayList<Node>();
	private float[][] CDT;
	private float[] marginalD;
	
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
	public void setCDT(int offset, float[] cd) throws Exception{
		if(cd.length==nState)
			CDT[offset]=cd;
		else{
			throw new Exception("Length of argmuent("+cd.length+") do not fit node requirement("+nState+")");
		}
	}
	public float[] getCDT(int offset){
		return CDT[offset];
	}
	public void setCDT(int[] parentValues, float[] cd) throws Exception{
		if(cd.length==nState)
			CDT[encodeParent(parentValues)]=cd;
		else{
			throw new Exception("Length of argmuent("+cd.length+") do not fit node requirement("+nState+")");
		}
	}
	public float[] getCDT(int[] parentsValues){
		int code=encodeParent(parentsValues);
		if(CDT[code]!=null)
			return CDT[code];
		return CDT[code]=marginalD;
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
	
	//Getters & Setters:
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
	public float[] getMarginalD() {
		return marginalD;
	}
	public void setMarginalD(float[] marginalD) {
		this.marginalD = marginalD;
	}

}
