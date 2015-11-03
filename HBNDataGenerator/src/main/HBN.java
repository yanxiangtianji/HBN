package main;

import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Random;

import org.apache.commons.math3.distribution.NormalDistribution;

public class HBN {
	int nLevel;
	int nNodes;
	int[] nEachLevel;
	int[] nBeforeLevel;
	Node[][] nodes;
	Node[] nodes_linear;

	public HBN(int level, int numNodes, int[] nEachLevel,double parentMeanRatio,
			double contextStateMean, double contextStateVar, int linkState) {
		nLevel=level;
		nNodes=numNodes;
		this.nEachLevel=nEachLevel;
		nodes=new Node[nLevel][];
		for(int i=0;i<nLevel;i++)
			nodes[i]=new Node[nEachLevel[i]];
		nodes_linear=new Node[numNodes];
		nBeforeLevel=new int[nLevel];
		nBeforeLevel[0]=0;
		for(int i=1;i<nLevel;i++)
			nBeforeLevel[i]=nBeforeLevel[i-1]+nEachLevel[i-1];
		Random rand=new Random();
		int linear_p=0;
		NormalDistribution randNormal=new NormalDistribution(contextStateMean,contextStateVar);
		for(int i=0;i<nodes[0].length;i++){
			Node n=generateNode("cxt_"+i,0,0,Math.max(2, (int)randNormal.sample()),rand);
			nodes[0][i]=n;
			nodes_linear[linear_p++]=n;
		}
		for(int l=1;l<nodes.length;l++){
			//-3sigma ~ +3sigma -> 99.73%
			double parentVar=Math.min(parentMeanRatio, 1.0-parentMeanRatio)*parentMeanRatio/3;
			randNormal=new NormalDistribution(nBeforeLevel[l]*parentMeanRatio, parentVar);
			for(int i=0;i<nodes[l].length;i++){
				int nParent=Math.max(1, Math.min((int) randNormal.sample(),nBeforeLevel[l]));
				String name="tfc_"+(l-1)+"_"+i;
				Node n=generateNode(name,l,nParent,linkState,rand);
				nodes[l][i]=n;
				nodes_linear[linear_p++]=n;
			}
		}
	}
	
	private Node generateNode(String name,int level,int nParent,int nState,Random rand){
		if(level==0)
			return new Node(name,nState,level,null,rand);
		Node[] parents=new Node[nParent];
		HashSet<Integer> used=new HashSet<Integer>(nParent);
		int limit=nBeforeLevel[level];
		for(int i=0;i<nParent;i++){
			int p=rand.nextInt(limit);
			while(used.contains(p)==true)
				p=rand.nextInt();
			parents[i]=nodes_linear[p];
		}
		return new Node(name,nState,level,parents,rand);
	}
	
	public int[] getnStateLevel(int levell){
		int[] res=new int[nodes[0].length];
		for(int i=0;i<nodes[0].length;i++)
			res[i]=nodes[0][i].nState;
		return res;
	}

	public void setState(int[] level0, int[]... levelRest){
		//first layer is mandatory
		for(int i=0;i<nodes[0].length;i++)
			nodes[0][i].state=level0[i];
		//other layer is optional
		for(int l=0;l<levelRest.length;l++){
			for(int i=0;i<nodes[l+1].length;i++){
				nodes[l+1][i].state=levelRest[l][i];
			}
		}
	}
	public void setState(int layer,int offer, int value){
		nodes[layer][offer].state=value;
	}
	
	public void print(PrintWriter wr,String sepper){
		StringBuilder sb=new StringBuilder();
		for(Node n:nodes_linear){
			n.calState();
			sb.append(n.state+sepper);
		}
		sb.setCharAt(sb.length()-1, '\n');
		wr.print(sb.toString());
	}
	
	public void printName(PrintWriter wr,String sepper){
		StringBuilder sb=new StringBuilder();
		for(Node n: nodes_linear){
			sb.append(n.name+sepper);
		}
		sb.setCharAt(sb.length()-1, '\n');
		wr.print(sb.toString());
	}
	
	public void printConf(PrintWriter wr,String sepper){
		wr.print(nNodes);
		wr.print('\n');
		StringBuilder sb=new StringBuilder();
		for(Node n: nodes_linear){
			sb.append(n.nState+sepper);
		}
		wr.print(sb.toString());
	}
	
	public void printNode(PrintWriter wr){
		wr.print(nNodes);
		wr.print('\n');
		for(Node n: nodes_linear){
			wr.println(n.name+"\t"+n.nState);
		}
	}
	
	public void printKnowledge(PrintWriter wr){
		for(Node n: nodes_linear){
			wr.println(n.name+"\t"+n.level);
		}
	}

}
