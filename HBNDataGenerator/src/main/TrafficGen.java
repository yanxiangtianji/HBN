package main;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Random;
import java.util.Scanner;

public class TrafficGen {
	HBN hbn;
	int nLevel;
	int nNode;
	int[] nEachLevel;
	Random rand;

	public TrafficGen(int nFuture,int nLink,int nContext,double parentMeanRatio,
			double contextStateMean, double contextStateVar, int linkState) {
		nLevel=nFuture+2;
		nNode=nContext+(nFuture+1)*nLink;
		nEachLevel=new int[nLevel];
		nEachLevel[0]=nContext;
		for(int i=1;i<nLevel;i++)
			nEachLevel[i]=nLink;
		hbn=new HBN(nLevel,nNode,nEachLevel,parentMeanRatio,contextStateMean,contextStateVar,linkState);
		rand=new Random();
	}
	
	public void setState(){
		int[] context=new int[nEachLevel[0]];
		int[] current=new int[nEachLevel[1]];
		for(int i=0;i<nEachLevel[0];i++)
			context[i]=rand.nextInt();
		for(int i=0;i<nEachLevel[1];i++)
			context[i]=rand.nextInt();
		hbn.setState(context, current);
	}
	
	public void output(String folder,int nPiece,int nPerFile) throws IOException{
		int count=0;
		int fileCount=0;
		PrintWriter wr=null;
		int[] given=new int[nEachLevel[0]];
		int[] nState=hbn.getnStateLevel(0);
		while(nPiece-->0){
			if(wr==null || count==nPerFile){
				if(wr!=null){
					System.out.println(count+" line(s) output.");
					wr.close();
				}
				System.out.print("Outputting file "+fileCount+": ");
				wr=new PrintWriter(new BufferedWriter(new FileWriter(folder+"/"+ fileCount +".csv")));
				++fileCount;
				count=0;
			}
			++count;
			for(int i=0;i<given.length;i++)
				given[i]=rand.nextInt(nState[i]);
			hbn.setState(given);
			hbn.print(wr,",");
		}
		if(wr!=null){
			System.out.println(count+" line(s) output.");
			wr.close();
		}
	}
	
	public void outputToOne(String folder,int nPiece) throws IOException{
		PrintWriter wr=new PrintWriter(new BufferedWriter(new FileWriter(folder+"/data.csv")));
		hbn.printName(wr, ",");
		int[] given=new int[nEachLevel[0]];
		int[] nState=hbn.getnStateLevel(0);
		for(int count=0;count<nPiece;count++){
			for(int i=0;i<given.length;i++)
				given[i]=rand.nextInt(nState[i]);
			hbn.setState(given);
			hbn.print(wr,",");
		}
		wr.close();
	}
	

	public void printCSVHead(String filename) throws IOException {
		PrintWriter wr=new PrintWriter(new FileWriter(filename));
		hbn.printName(wr, ",");
		wr.close();
	}

	public void printCSVConf(String filename) throws IOException {
		PrintWriter wr=new PrintWriter(new FileWriter(filename));
		hbn.printConf(wr, " ");
		wr.close();
	}

	public void printNode(String filename) throws IOException {
		PrintWriter wr=new PrintWriter(new FileWriter(filename));
		hbn.printNode(wr);
		wr.close();
	}

	public void printKnowledge(String filename) throws IOException {
		PrintWriter wr=new PrintWriter(new FileWriter(filename));
		hbn.printKnowledge(wr);
		wr.close();
	}
	
	public static void outputForBatch(String folder, TrafficGen obj, int nPiece) throws IOException{
		System.out.println("Outputting conf...");
		File path=new File(folder);
		path.delete();
		path.mkdirs();
		path=null;
		obj.printNode(folder+"/node_traffic.txt");
		obj.printKnowledge(folder+"/knowledge_traffic.txt");
		
		System.out.println("Outputting data...");
		obj.outputToOne(folder,nPiece);
		System.out.println("Finish.");
	}
	
	public static void outputForDistributed(String folder, TrafficGen obj, int nPiece, int nPerFile) throws IOException{
		System.out.println("Outputting conf...");
		File confPath=new File(folder+"/conf/");
		confPath.delete();
		confPath.mkdirs();
		confPath=null;
		obj.printCSVConf(folder+"/conf/csvconf.txt");
		obj.printCSVHead(folder+"/conf/csvhead.csv");
		obj.printNode(folder+"/conf/node.txt");
		obj.printKnowledge(folder+"/conf/knowledge.txt");
		
		System.out.println("Outputting data...");
		File dataPath=new File(folder+"/input/");
		dataPath.delete();
		dataPath.mkdirs();
		dataPath=null;
		obj.output(folder+"/input/", nPiece, nPerFile);
		System.out.println("Finish.");
	}
	
	public static void main(String[] args) throws IOException {
		String baseDir="../tmp/Gen/";
		
		System.out.print("Please input the name of configure file: ");
		Scanner scn=new Scanner(System.in);
		String confName=scn.next();
		scn.close();
		String baseName=confName.substring(0,confName.lastIndexOf('.'));
		
		System.out.println("Loading conf...");
		scn=new Scanner(new FileReader(baseDir+confName));
		//parameters for hbn
		int nContext=scn.nextInt();
		int nLink=scn.nextInt();
		int nFuture=scn.nextInt();
		double parentMeanRatio=scn.nextDouble();
		double contextStateMean=scn.nextDouble();
		double contextStateVar=scn.nextDouble(); 
		int linkState=scn.nextInt();
		//parameters for generating
		int nPiece=scn.nextInt();
		int nPerFile=scn.nextInt();
		scn.close();
		
		System.out.println("Constructing...");
//		Node.setMinMaxF(-1.0f, 1.0f);
		TrafficGen obj=new TrafficGen(nFuture,nLink,nContext,parentMeanRatio,contextStateMean,contextStateVar,linkState);
		
//		outputForBatch(baseDir+baseName+"/",obj,nPiece);
		outputForDistributed(baseDir+baseName+"/",obj,nPiece,nPerFile);
	}

	
	
}
