package hbn;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.Scanner;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileUtil;
import org.apache.hadoop.fs.Path;

import Entity.Pair;
import util.PartFileFilter;
import zt.CSVFamScore;
import zt.CSVFamScoreParam;

public class Network {
	private int N;
	private ArrayList<Node> nodes=new ArrayList<Node>();
	private HashMap<String,Node> name2node=new HashMap<String,Node>();
	private HashMap<String,Integer> name2off=new HashMap<String,Integer>();
	private ArrayList<Node> csv2node=new ArrayList<Node>();	//csv offset to node
	private ArrayList<Integer> csv2off=new ArrayList<Integer>();	//csv offset to node offset
	private ArrayList<ArrayList<Integer>> nodeByLayer=new ArrayList<ArrayList<Integer>>();
	private FileSystem hdfs;
	
	Network(FileSystem hdfs, String nodeFile, String knowledgeFile) throws IOException{
		this.hdfs=hdfs;
		Scanner s_n=new Scanner(hdfs.open(new Path(nodeFile)));
		N=s_n.nextInt();
		String[] names=new String[N];
		int[] nStates=new int[N];
		int[] layers=new int[N];
		for(int i=0;i<N;i++){	//load name & nState
			String name=s_n.next();
			names[i]=name;
			nStates[i]=s_n.nextInt();
			name2off.put(name,i);
		}
		s_n.close();
		Scanner s_k=new Scanner(hdfs.open(new Path(knowledgeFile)));
		for(int i=0;i<N;i++){	//load name & layer
			layers[name2off.get(s_k.next())]=s_k.nextInt();
		}
		s_k.close();
		for(int i=0;i<N;i++){	//generate nodes
			Node n=new Node(names[i],nStates[i],layers[i]);
			nodes.add(n);
			name2node.put(names[i], n);
		}
		setNodeByLayer();
	}
	
	private void setNodeByLayer(){	//arrange nodes on each layer
		for(int i=0;i<N;i++){
			int layer=nodes.get(i).getLayer();
			while(nodeByLayer.size()<=layer){	//format the raw container
				nodeByLayer.add(new ArrayList<Integer>());
			}
			nodeByLayer.get(layer).add(i);
		}
	}
	private void setCSVMapping(String csvHeadFile) throws IOException{	//process csv position and inner position
		BufferedReader br=new BufferedReader(new InputStreamReader(hdfs.open(new Path(csvHeadFile))));
		String line=br.readLine();
		br.close();
		for(String str : line.split(",")){
			csv2node.add(name2node.get(str));
			csv2off.add(name2off.get(str));
		}
	}
	private void clearCSVMapping(){
		csv2node=null;
		csv2off=null;
	}
	private void generateCSVConfFile(String csvConfFile) throws IOException{
		PrintWriter pw=new PrintWriter(new BufferedWriter(new OutputStreamWriter(hdfs.create(new Path(csvConfFile)))));
		pw.println(N);
		for(Node n : csv2node){
			pw.print(n.getnState());
			pw.print(" ");
		}
		pw.close();
	}
	
	public void greedyLearning(String csvHeadFile, String input, String csvConfFile
			, String baseOutput, double threshold) throws Exception{
		//prepare:
		setCSVMapping(csvHeadFile);		//map location in csv file to inner position
		generateCSVConfFile(csvConfFile);	//generate the csv configuration file
		//prepare param:
		CSVFamScoreParam param=new CSVFamScoreParam();
		param.setCsvConfFile(csvConfFile);
		param.setInput(input);
		ArrayList<Integer> possibleParents=new ArrayList<Integer>();
		for(int i: nodeByLayer.get(0))	//finish preparing initial parents
			possibleParents.add(i);
		//start working:
		for(int layer=1;layer<nodeByLayer.size();layer++){
			ArrayList<Integer> thisLayer=nodeByLayer.get(layer);
			param.setPossible(possibleParents);
			for(int i=0;i<thisLayer.size();i++){
				greedyLearningOne(thisLayer.get(i), param, baseOutput, threshold);
			}
			possibleParents.addAll(nodeByLayer.get(layer));
		}
		clearCSVMapping();
	}
	private void greedyLearningOne(int specified, CSVFamScoreParam param, String baseOutput,
			double threshold) throws Exception{
		CSVFamScore f=new CSVFamScore();
		CSVFamScoreParam p=param.clone();
		p.setSpecified(specified);
//		ArrayList<Integer> given=new ArrayList<Integer>();
		baseOutput=baseOutput+"/famscore_"+specified;
		Path outputPath=new Path(baseOutput);
		if(hdfs.exists(outputPath))
			hdfs.delete(outputPath, true);
		outputPath=null;
		baseOutput=baseOutput+"/give_";
		
		double last=-Double.MAX_VALUE, current=-Double.MAX_VALUE;
		int maxIteration=param.getPossible().size();
		while(maxIteration-->0 && 
				(current==-Double.MAX_VALUE || current-last>=threshold)){
			//calculate the famscores
//			p.setGiven(given);
			String output=baseOutput+p.getGiven().toString().replaceAll("[\\[\\] ]", "");
			p.setOutput(output);
			f.configure(p);
			f.run();
			//find the node with largest famscore and connect it with current node
			Pair<Integer,Double> famRes=findLargestFamScore(output);
			last=current;
			current=famRes.second.doubleValue();
			int parent=csv2off.get(famRes.first.intValue());
//			given.add(parent);
			p.addGiven(parent);
			p.removePossible(parent);
			nodes.get(specified).addParent(nodes.get(parent));
		}
	}
	/**
	 * The returned node id is the offset of the node in the csv file. 
	 * 
	 * @param output -> the folder to scan
	 * @return the node id with largest FamScore
	 * @throws IOException
	 */
	private Pair<Integer,Double> findLargestFamScore(String output) throws IOException{
		int res=-1;
		double largest=-Double.MAX_VALUE;
		Path[] listedPaths=FileUtil.stat2Paths(hdfs.listStatus(new Path(output), new PartFileFilter()));
		for(Path file: listedPaths){
			Scanner scn=new Scanner(hdfs.open(file));
			while(scn.hasNext()){
				int n=scn.nextInt();
				double s=scn.nextDouble();
				if(s>largest){
					largest=s;
					res=n;
				}
			}
			scn.close();
		}
		return new Pair<Integer,Double>(res,largest);
	}
	
	public void calDistribution(){
		
	}
	public void outputStructure(String structureFile,boolean withDistribution, boolean withMarginal) throws IOException{
		PrintWriter pw=new PrintWriter(new BufferedWriter(new OutputStreamWriter(hdfs.create(new Path(structureFile)))));		
		for(Node n: nodes){
			ArrayList<Node> parents=n.getParents();
			pw.println(n.getName()+" "+parents.size());
			if(parents.size()==0)
				continue;
			pw.print(parents.get(0).getName());
			for(int i=1;i<parents.size();i++){
				pw.print(" ");
				pw.print(parents.get(i));
			}
			pw.print("\n");
		}
		pw.close();
	}

	//Main:
	public static void main(String[] args) throws Exception{
		FileSystem hdfs=FileSystem.get(new Configuration());
		HashMap<String,String> properties=new HashMap<String,String>();
		String PREFIX="traffic";
		properties.put("nodeFile", PREFIX+"/conf/node.txt");
		properties.put("knowledgeFile", PREFIX+"/conf/knowledge.txt");
		properties.put("csvHeadFile", PREFIX+"/conf/csvhead.csv");
		properties.put("csvConfFile", PREFIX+"/conf/csvconf.txt");
		properties.put("input", PREFIX+"/input");
		properties.put("output", PREFIX+"/output/");
		properties.put("structureBriefFile", PREFIX+"/structure_brief.txt");
		//for local debug
		for(Entry<String,String> entry: properties.entrySet())
			entry.setValue("../tmp/"+entry.getValue());
		//run:
		Network net=new Network(hdfs,properties.get("nodeFile"), properties.get("knowledgeFile"));
		net.greedyLearning(properties.get("csvHeadFile"),properties.get("input"),
				properties.get("csvConfFile"),properties.get("output"),50.0);
		net.outputStructure(properties.get("structureBriefFile"), false, false);
	}
	
}
