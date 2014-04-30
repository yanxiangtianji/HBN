package hbn;

import java.util.ArrayList;
import java.util.Comparator;

public class Node implements Comparator<Node>{
	private String name;
	private int layer;
	private int nState;
	private ArrayList<Node> parents=new ArrayList<Node>();
	
	Node(String name,int nState){
		this(name,nState,0);
	}
	Node(String name,int nState,int layer){
		this.name=name;
		this.nState=nState;
		this.layer=layer;
	}
	
	public Node getParent(int idx){
		return parents.get(idx);
	}
	public void addParent(Node n){
		parents.add(n);
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

	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		return name.hashCode();
	}
	/* (non-Javadoc)
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
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
	
}
