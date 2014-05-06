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

import util.PartFileFilter;
import zt.CSVCDParam;
import zt.CSVConditionalDistribution;
import zt.CSVFamScore;
import zt.CSVFamScoreParam;
import Entity.Pair;

public class Network {
	private int N;
	private ArrayList<Node> nodes=new ArrayList<Node>();
	private HashMap<String,Node> name2node=new HashMap<String,Node>();
	private HashMap<String,Integer> name2off=new HashMap<String,Integer>();
	private HashMap<String,Integer> name2csv=new HashMap<String,Integer>();	//name to csv offset
	private ArrayList<Node> csv2node=new ArrayList<Node>();	//csv offset to node
	private ArrayList<Integer> csv2off=new ArrayList<Integer>();	//csv offset to node offset
	private ArrayList<ArrayList<Integer>> nodeByLayer=new ArrayList<ArrayList<Integer>>();
	private FileSystem hdfs;
	
	//initial (generate node, arrange by layer)
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
	//initial (arrange by layer)
	private void setNodeByLayer(){	//arrange nodes on each layer
		for(int i=0;i<N;i++){
			int layer=nodes.get(i).getLayer();
			while(nodeByLayer.size()<=layer){	//format the raw container
				nodeByLayer.add(new ArrayList<Integer>());
			}
			nodeByLayer.get(layer).add(i);
		}
	}
	
	/**
	 * Initialize csv information: set inner parameters, output a csv configuration file. 
	 * 
	 * @param csvHeadFile:	input csv file with the head line.
	 * @param csvConfFile:	output a csv configuration file used for Hadoop csv procedures
	 * @throws IOException
	 */
	public void setCSVFormat(String csvHeadFile, String csvConfFile) throws IOException{
		System.out.println("Start to set csv format information.");
		setCSVMapping(csvHeadFile);
		generateCSVConfFile(csvConfFile);
	}
	//map location in csv file to inner position
	public void setCSVMapping(String csvHeadFile) throws IOException{	//process csv position and inner position
		BufferedReader br=new BufferedReader(new InputStreamReader(hdfs.open(new Path(csvHeadFile))));
		String line=br.readLine();
		br.close();
		int p=0;
		for(String str : line.split(",")){
			csv2node.add(name2node.get(str));
			csv2off.add(name2off.get(str));
			name2csv.put(str, p++);
		}
	}
	//generate the csv configuration file
	public void generateCSVConfFile(String csvConfFile) throws IOException{
		System.out.println("Start to output csv configuration file used for hadoop jobs.");
		PrintWriter pw=new PrintWriter(new BufferedWriter(new OutputStreamWriter(hdfs.create(new Path(csvConfFile)))));
		pw.println(N);
		for(Node n : csv2node){
			pw.print(n.getnState());
			pw.print(" ");
		}
		pw.close();
	}
	public void clearCSVMapping(){
		csv2node=null;
		csv2off=null;
		name2csv=null;
	}
	public int map2csvOffset(Node n){
		return name2csv.get(n.getName());
	}
	public int map2csvOffset(int innerOffset){
		return name2csv.get(nodes.get(innerOffset).getName());
	}
	
	/**
	 * Greedy learning on given csv files
	 * 
	 * @param csvHeadFile;	file containing the csv head line. Used for mapping csv offset to inner offset 
	 * @param input;	folder containing input csv files
	 * @param csvConfFile;	the number of node and the number of states for each node
	 * @param baseOutput
	 * @param threshold
	 * @throws Exception
	 */
	public void greedyLearning(String input, String csvConfFile
			, String baseOutput, double threshold) throws Exception{
		System.out.println("Start the greedy learning.");
		//prepare:
		CSVFamScoreParam param=new CSVFamScoreParam();
		param.setCsvConfFile(csvConfFile);
		param.setInput(input);
		ArrayList<Integer> possibleParentsInCSV=new ArrayList<Integer>();
		for(int i : nodeByLayer.get(0)){	//initialize parents
			possibleParentsInCSV.add(map2csvOffset(i));
		}
		//start working:
		for(int layer=1;layer<nodeByLayer.size();layer++){
			System.out.println("Processing layer "+layer+"...");
			ArrayList<Integer> thisLayer=nodeByLayer.get(layer);
			param.setPossible(possibleParentsInCSV);
			for(int i=0;i<thisLayer.size();i++){
				System.out.println("Processing node "+i+"...");
				greedyLearningOne(nodes.get(thisLayer.get(i)), param, baseOutput, threshold);
				System.out.println("Finish processing node "+i+".");
			}
			for(int i : thisLayer)
				possibleParentsInCSV.add(map2csvOffset(i));
			System.out.println("Finish processing layer "+layer+".");
		}
	}
	private void greedyLearningOne(Node specifiedNode, CSVFamScoreParam param, String baseOutput,
			double threshold) throws Exception{
//		Node specifiedNode=nodes.get(csv2off.get(specified));
		int specified=name2off.get(specifiedNode.getName());
		CSVFamScoreParam p=param.clone();
		p.setSpecified(specified);
//		ArrayList<Integer> given=new ArrayList<Integer>();
		baseOutput=baseOutput+"/famscore_"+specified;
		Path outputPath=new Path(baseOutput);
		if(hdfs.exists(outputPath))
			hdfs.delete(outputPath, true);
		outputPath=null;
		baseOutput=baseOutput+"/give_";
		
		CSVFamScore f=new CSVFamScore();
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
			int parentInCSV=famRes.first.intValue();
			int parentInHBN=csv2off.get(parentInCSV);
			p.movePossible2Given(parentInCSV);
			specifiedNode.addParent(nodes.get(parentInHBN));
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
	
	/**
	 * calculate conditional distribution table with given data and given edges.
	 * 
	 * @param input;	folder for given data files
	 * @param briefStructureFile;	brief structure file described in csv offset.
	 *  If it doesn't exist, create one with current information.
	 * @param csvConfFile;	configuration file about csv file information.
	 *  if it doesn't exist, throw an exception.
	 * @param distributionFolder;	the Hadoop output folder for distribution calculation 
	 * @throws Exception; 1, csvConfFile doesn't exist. 2, inner Hadoop job throws exceptions. 
	 */
	public void calDistribution(String input, String briefStructureFile, 
			String csvConfFile, String distributionFolder) throws Exception{
		if(!hdfs.exists(new Path(csvConfFile)))
			throw new Exception("csv configuration file does not exists!");
		if(!hdfs.exists(new Path(briefStructureFile)))	
			outputBriefStructureWithCSVoff(briefStructureFile);
		CSVCDParam param=new CSVCDParam(csvConfFile, input, distributionFolder, briefStructureFile);
		CSVConditionalDistribution cd=new CSVConditionalDistribution(param);
		Path df=new Path(distributionFolder);
		if(hdfs.exists(df))
			hdfs.delete(df,true);
		df=null;
		System.out.println("Start to calculate conditional distribution tables.");
		cd.run();
		System.out.println("Start to load conditional distribution tables and generate marginal distributions.");
		loadDistributionToNode(distributionFolder);
	}
	//load distribution from hadoop output file
	private void loadDistributionToNode(String distributionFolder) throws IOException{
		Path[] listedPaths=FileUtil.stat2Paths(hdfs.listStatus(new Path(distributionFolder), new PartFileFilter()));
		for(Node n: nodes){
			n.initCDT();
		}
		int[][] countMarginal=new int[nodes.size()][];
		for(int i=0;i<nodes.size();i++){
			countMarginal[i]=new int[nodes.get(i).getnState()];
		}
		for(Path file : listedPaths){
			BufferedReader br=new BufferedReader(new InputStreamReader(hdfs.open(file)));
			String temp=null;
			while((temp=br.readLine())!=null){
				String[] line=temp.split("[\t,]");
				int innerIDX=csv2off.get(Integer.parseInt(line[0]));
				Node n=nodes.get(innerIDX);
				int off=Integer.parseInt(line[1]);
				float[] arr=new float[n.getnState()];
				int sum=0;
				for(int i=2;i<line.length;i++){
					int t=Integer.parseInt(line[i]);
					arr[i-2]=t;
					sum+=t;
					countMarginal[innerIDX][i-2]+=t;
				}
				for(int i=0;i<arr.length;i++)
					arr[i]/=sum;
				try {
					if(n.getParents().size()!=0)
						n.setCDT(off, arr);	//for normal nodes, set conditional distribution table entry
					else
						n.setMarginalD(arr);	//for layer-0 nodes, set marginal distribution
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
			br.close();
		}//end for cdt (and md for layer-0 nodes)
		for(int i=0;i<countMarginal.length;++i){
			Node n=nodes.get(i);
			if(n.getParents().size()==0)
				continue;
			int sum=0;
			for(int j=0;j<countMarginal[i].length;++j)
				sum+=countMarginal[i][j];
			float[] md=new float[countMarginal[i].length];
			for(int j=0;j<countMarginal[i].length;++j)
				md[j]=countMarginal[i][j]/(float)sum;
			nodes.get(i).setMarginalD(md);
		}//end for md
	}
	
	/**
	 * load structures from regular file with given parameters
	 * 
	 * @param structureFile
	 * @param withDistribution
	 * @param withMarginal
	 * @throws IOException
	 */
	public void loadStructure(String structureFile, boolean withDistribution, boolean withMarginal) throws IOException{
		Scanner scn=new Scanner(new BufferedReader(new InputStreamReader(hdfs.open(new Path(structureFile)))));
		for(int i=0;i<nodes.size();i++){
			Node n=name2node.get(scn.next());
			int numParent=scn.nextInt();
			for(int j=0;j<numParent;j++){
				String parent=scn.next();
				n.addParent(name2node.get(parent));
			}
		}
		scn.close();
	}
	
	//output structures
	public void outputBriefStructureWithName(String structureFile) throws IOException{
		System.out.println("Start to output brief structure via node name.");
		PrintWriter pw=new PrintWriter(new BufferedWriter(new OutputStreamWriter(hdfs.create(new Path(structureFile)))));		
		for(Node n: nodes){
			ArrayList<Node> parents=n.getParents();
			pw.println(n.getName()+" "+parents.size());
			if(parents.size()==0)
				continue;
			pw.print(parents.get(0).getName());
			for(int i=1;i<parents.size();i++){
				pw.print(" ");
				pw.print(parents.get(i).getName());
			}
			pw.print("\n");
		}
		pw.close();
	}
	public void outputBriefStructureWithCSVoff(String structureFile) throws IOException{
		System.out.println("Start to output brief structure via offset in the csv file.");
		PrintWriter pw=new PrintWriter(new BufferedWriter(new OutputStreamWriter(hdfs.create(new Path(structureFile)))));		
		for(Node n: nodes){
			ArrayList<Node> parents=n.getParents();
			pw.println(name2csv.get(n.getName())+" "+parents.size());
			if(parents.size()==0)
				continue;
			pw.print(name2csv.get(parents.get(0).getName()));
			for(int i=1;i<parents.size();i++){
				pw.print(" ");
				pw.print(name2csv.get(parents.get(i).getName()));
			}
			pw.print("\n");
		}
		pw.close();
	}
	public void outputStructure(String structureFile, boolean withMarginal) throws IOException{
		System.out.println("Start to output full structure via node name, "
				+(withMarginal?"with":"with out")+" marginal distribution.");
		PrintWriter pw=new PrintWriter(new BufferedWriter(new OutputStreamWriter(hdfs.create(new Path(structureFile)))));		
		for(Node n: nodes){
			ArrayList<Node> parents=n.getParents();
			pw.println(n.getName()+" "+parents.size());
			if(withMarginal){
				StringBuilder sb=new StringBuilder();
				for(float f : n.getMarginalD()){
					sb.append(f);
					sb.append(" ");
				}
				sb.delete(sb.length()-1, sb.length());	//remove the last " "
				pw.println(sb.toString());
			}
			if(parents.size()==0)
				continue;	//no parents
			pw.print(parents.get(0).getName());
			for(int i=1;i<parents.size();i++){
				pw.print(" ");
				pw.print(parents.get(i).getName());
			}
			pw.print("\n");
			//output distribution
			for(float[] md : n.getCDT()){
				pw.print(md[0]);
				for(int i=1;i<md.length;++i){
					pw.print(" ");
					pw.print(md[i]);
				}
				pw.print("\n");
			}
		}
		pw.close();		
	}

	//Main:
	public static void main(String[] args) throws Exception{
		FileSystem hdfs=FileSystem.get(new Configuration());
		HashMap<String,String> properties=new HashMap<String,String>();
		String PREFIX="../tmp/traffic2/";
		properties.put("nodeFile", PREFIX+"conf/node.txt");
		properties.put("knowledgeFile", PREFIX+"conf/knowledge.txt");
		properties.put("csvHeadFile", PREFIX+"conf/csvhead.csv");
		properties.put("csvConfFile", PREFIX+"conf/csvconf.txt");
		properties.put("csvFolder", PREFIX+"input");
		properties.put("famScoreFolder", PREFIX+"output_famscore/");
		properties.put("structureBriefFile", PREFIX+"structure_brief.txt");
		properties.put("structureCSVBriefFile", PREFIX+"structure_brief_csv.txt");
		properties.put("distributionFolder", PREFIX+"output_distribution");
		properties.put("structureFile",PREFIX+"structure.txt");
		//for local debug
		for(Entry<String,String> entry: properties.entrySet())
			entry.setValue("../tmp/"+entry.getValue());
		//run:
		Network net=new Network(hdfs,properties.get("nodeFile"), properties.get("knowledgeFile"));
		net.setCSVFormat(properties.get("csvHeadFile"),properties.get("csvConfFile"));
		net.greedyLearning(properties.get("csvFolder"),properties.get("csvConfFile"),
				properties.get("famScoreFolder"),500.0);
//		net.loadStructure(properties.get("structureBriefFile"), false, false);
		
		net.outputBriefStructureWithName(properties.get("structureBriefFile"));
		net.outputBriefStructureWithCSVoff(properties.get("structureCSVBriefFile"));
		net.calDistribution(properties.get("csvFolder"), properties.get("structureCSVBriefFile"),
				properties.get("csvConfFile"), properties.get("distributionFolder"));
		net.outputStructure(properties.get("structureFile"),true);
		
		System.out.println("Finished.");
	}
	
}
