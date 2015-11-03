package main;

import java.util.Random;

public class Node {
	//dynamic
	int state;
	//static
	int nState;
	Node[] parents;
	String name;
	int level;
	//random
	float min,max,scale;
	float[] w1;
	float[][] w2;

	static float MIN_W=-1.0f, MAX_W=1.0f,RANGE_W=2.0f;
	static void setMinMaxF(float min,float max){
		MIN_W=min;
		MAX_W=max;
		RANGE_W=MAX_W-MIN_W;
	}

	static private float genWeight(float v){
		return (v*RANGE_W)+MIN_W;
	}
	
	
	private void init(Random rand){
		int len=parents.length;
		w1=new float[len];
		w2=new float[len][len];
		min=max=0;
		for(int i=0;i<len;i++){
			w1[i]=genWeight(rand.nextFloat());
			int k=parents[i].nState-1;
			float t=w1[i]*k;
			if(t<=0)	min+=t;
			else	max+=t;
			for(int j=0;j<len;j++){
				w2[i][j]=genWeight(rand.nextFloat());
				t=w2[i][j]*k*(parents[j].nState-1);
				if(t<=0)	min+=t;
				else	max+=t;
			}
		}
		scale=1.0f/(max-min)*nState;
	}
	public Node(String name,int nState,int level,Node[] parents,  Random rand) {
		this.name=name;
		this.nState=nState;
		this.level=level;
		this.parents=parents;
		if(parents!=null)
			init(rand);
	}
	
	public int calState(){
		if(parents==null)
			return state;
		float res=0.0f;
		int len=parents.length;
		for(int i=0;i<len;i++){
			float t=w1[i];
			for(int j=0;j<len;j++)
				t+=w2[i][j]*parents[j].state;
			res+=t*parents[i].state;
		}
		state=(int) ((res-min)*scale);
		return state;
	}

}
