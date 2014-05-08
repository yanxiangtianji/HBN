package hbn;

import hadoop.CSVCDParam;
import hadoop.CSVFamScoreParam;
import hadoop.job.CSVConditionalDistribution;
import hadoop.job.CSVFamScore;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Scanner;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileUtil;
import org.apache.hadoop.fs.Path;

import util.PartFileFilter;
import Entity.Pair;

public class Network {
//	private int N;
	private ArrayList<Node> nodes=new ArrayList<Node>();
	private HashMap<String,Node> name2node=new HashMap<String,Node>();
	private HashMap<String,Integer> name2off=new HashMap<String,Integer>();
	private HashMap<String,Integer> name2csv=new HashMap<String,Integer>();	//name to csv offset
	private ArrayList<Node> csv2node=new ArrayList<Node>();	//csv offset to node
	private ArrayList<Integer> csv2off=new ArrayList<Integer>();	//csv offset to node offset
	private ArrayList<ArrayList<Integer>> nodeByLayer=new ArrayList<ArrayList<Integer>>();
	private FileSystem hdfs;
	
	//initial (generate node, arrange by layer)
	public Network(FileSystem hdfs, String nodeFile, String knowledgeFile) throws IOException{
		this.hdfs=hdfs;
		Scanner s_n=new Scanner(hdfs.open(new Path(nodeFile)));
		int N=s_n.nextInt();
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
		for(int i=0;i<nodes.size();i++){
			int layer=nodes.get(i).getLayer();
			while(nodeByLayer.size()<=layer){	//format the raw container
				nodeByLayer.add(new ArrayList<Integer>());
			}
			nodeByLayer.get(layer).add(i);
		}
	}
	/**
	 * Copy construct a new network as the given reference network. 
	 * But the new network is memory independent to the old one, i.e. changing one network cannot affect the other one.
	 * 
	 * @param net
	 * @param copyStructure	whether to copy the parents relationship of each node
	 * @param copyDistribution	whether to copy the CDT and MD of each node. Matters when copyStructure is true.
	 * @throws IOException
	 */
	@SuppressWarnings("unchecked")
	public Network(Network net, boolean copyStructure, boolean copyDistribution) throws IOException{
		this.hdfs=net.hdfs;
//		N=net.N;
		//nodes:
		nodes=new ArrayList<Node>();
		HashMap<Node,Node> nodeMapping=new HashMap<Node,Node>();
		for(Node n:nodes){
			Node newn=new Node(n);
			nodes.add(newn);
			nodeMapping.put(n, newn);
		}		
		//name2off:
		name2off=(HashMap<String, Integer>) net.name2off.clone();
		//name2node:
		for(Entry<String,Node> entry:net.name2node.entrySet())
			name2node.put(entry.getKey(), nodeMapping.get(entry.getValue()));
		//nodeByLayer:
		nodeByLayer=(ArrayList<ArrayList<Integer>>) net.nodeByLayer.clone();
		//structure(parents):
		if(copyStructure){
			for(int i=0;i<nodes.size();i++){
				Node newn=nodes.get(i);
				Node oldn=net.nodes.get(i);
				for(Node p:oldn.getParents())
					newn.addParent(nodeMapping.get(p));
				//distributions (CDT and MD)
				if(copyDistribution){
					newn.setCDT(oldn.getCDT());
					newn.setDefaultMD(oldn.getDefaultMD());
				}
			}//nodes
		}//copyStructure
	}

	/**
	 * Initialize csv information: set inner parameters, output a csv configuration file. 
	 * 
	 * @param csvHeadFile	input csv file with the head line.
	 * @param csvConfFile	output a csv configuration file used for Hadoop csv procedures
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
		pw.println(nodes.size());
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
	
	//inner mappings:
	public Node map2node(String name){
		return name2node.get(name);
	}
	public Node map2node(int innerOffset){
		return nodes.get(innerOffset);
	}
	public Node map2nodeFromCSV(int csvOffset){
		return csv2node.get(csvOffset);
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
	 * @param csvHeadFile	file containing the csv head line. Used for mapping csv offset to inner offset 
	 * @param input	folder containing input csv files
	 * @param csvConfFile	the number of node and the number of states for each node
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
//		for(int layer=1;layer<2;layer++){
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
		int specified=map2csvOffset(specifiedNode);
		CSVFamScoreParam p=param.clone();
		p.setSpecified(specified);
		baseOutput=baseOutput+"/famscore_"+specified;
		Path outputPath=new Path(baseOutput);
		if(hdfs.exists(outputPath))
			hdfs.delete(outputPath, true);
		outputPath=null;
		baseOutput=baseOutput+"/give_";
		
		CSVFamScore f=new CSVFamScore();
		double last=-Double.MAX_VALUE, current=-Double.MAX_VALUE;
		int maxIteration=param.getPossible().size();
		int iteration=0;
		while(iteration++ < maxIteration && 
				(current==-Double.MAX_VALUE || current-last>=threshold)){
			//calculate the famscores
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
			//show status
			System.out.println("In loop "+iteration+", parent "+parentInCSV+"is choosen.");
			System.out.println("FamScore improvement is "+ (current-last));
		}
	}

	//find the node offset in csv file with largest FamScore in Hadoop output folder "output"
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
	 * @param input	folder for given data files
	 * @param briefStructureFile	brief structure file described in csv offset.
	 *  If it doesn't exist, create one with current information.
	 * @param csvConfFile	configuration file about csv file information.
	 *  if it doesn't exist, throw an exception.
	 * @param distributionFolder	the Hadoop output folder for distribution calculation 
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
						n.setCDT(off, arr, sum);	//for normal nodes, set conditional distribution table entry
					else
						n.setDefaultMD(arr);	//for layer-0 nodes, set marginal distribution
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
			n.setDefaultMD(md, sum);
		}//end for md
	}
	
	/**
	 * load structures from regular file with given parameters
	 * 
	 * @param structureFile
	 * @param withDistribution
	 * @param withMarginal
	 * @throws Exception 
	 */
	public void loadStructure(String structureFile, boolean withDistribution, boolean withMarginal) throws Exception{
		Scanner scn=new Scanner(new BufferedReader(new InputStreamReader(hdfs.open(new Path(structureFile)))));
		for(int i=0;i<nodes.size();i++){
			Node n=name2node.get(scn.next());
			int numParent=scn.nextInt();
			//marginal:
			if(withMarginal){
				float[] md=new float[n.getnState()];
				for(int j=0;j<n.getnState();++j)
					md[j]=scn.nextFloat();
				n.setDefaultMD(md);
			}
			if(numParent==0)
				continue;
			//parents:
			for(int j=0;j<numParent;j++){
				String parent=scn.next();
				n.addParent(name2node.get(parent));
			}
			//cdt:
			if(withDistribution){
				n.initCDT();
				int nCDT=scn.nextInt();
				while(nCDT-->0){
					int offset=scn.nextInt();
					float[] cpd=new float[n.getnState()];
					for(int j=0;j<n.getnState();++j)
						cpd[j]=scn.nextFloat();
					int sum=scn.nextInt();
					n.setCDT(offset, cpd, sum);
				}
			}
		}
		scn.close();
	}
	
	/**
	 * Output brief structure using node name
	 * 
	 * @param structureFile
	 * @throws IOException
	 */
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
	/**
	 * Output brief structure using node offset in the csv file
	 * 
	 * @param structureFile
	 * @throws IOException
	 */
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
	/**
	 * Output complete structure (with conditional distribution table (CDT)) using node name.
	 * 
	 * @param structureFile
	 * @param withMarginal	whether to output the marginal distribution of each node before the parent information
	 * @throws IOException
	 */
	public void outputStructure(String structureFile, boolean withMarginal) throws IOException{
		System.out.println("Start to output full structure via node name, "
				+(withMarginal?"with":"with out")+" marginal distribution.");
		PrintWriter pw=new PrintWriter(new BufferedWriter(new OutputStreamWriter(hdfs.create(new Path(structureFile)))));
		for(Node n: nodes){
			ArrayList<Node> parents=n.getParents();
			pw.println(n.getName()+" "+parents.size());
			//marginal distribution:
			if(withMarginal){
				for(float f : n.getDefaultMD()){
					pw.print(f);
					pw.print(" ");
				}
				pw.print(n.getOccurrenceAll());
				pw.print("\n");
			}
			//parents:
			if(parents.size()==0)	//no parents
				continue;
			pw.print(parents.get(0).getName());
			for(int i=1;i<parents.size();i++){
				pw.print(" ");
				pw.print(parents.get(i).getName());
			}
			pw.print("\n");
			//output distribution:
			int nCDT=n.getnCDT();
			pw.println(nCDT);
			for(int i=0;i<nCDT;++i){
				float[] md=n.getCDT(i);
				if(md==null)	//skip those entry without a cpd
					continue;
				pw.print(i);
				for(float f : md){
					pw.print(" ");
					pw.print(f);
				}
				pw.print(" ");
				pw.print(n.getOccurrenceCDT(i));
				pw.print("\n");
			}
		}
		pw.close();
	}
	
	
	/**
	 * Predict the queried nodes' states based on given states.
	 * 
	 * @param given	the map between given nodes and their states
	 * @param query	the list of queried nodes' name
	 * @return
	 */
	public Map<String,Integer> predict(Map<String,Integer> given, List<String> query){
		//get distributions
		Map<String,float[]> dis=predictDistribution(given, query);
		//get the states from the distributions
		HashMap<String,Integer> res=new HashMap<String,Integer>();
		for(Entry<String,float[]> entry : dis.entrySet()){
			res.put(entry.getKey(), Node.tellStateFromDistribution(entry.getValue()));
		}
		return res;
	}
	/**
	 * Predict the queried nodes' distribution based on given states.
	 * 
	 * @param given
	 * @param query
	 * @return
	 */
	public Map<String,float[]> predictDistribution(Map<String,Integer> given, List<String> query){
		HashMap<String,float[]> res=new HashMap<String,float[]>();
		if(query.size()==0)
			return res;
		//set given md
		for(Entry<String,Integer> entry : given.entrySet()){
			Node n=name2node.get(entry.getKey());
			n.setCacheMD(entry.getValue().intValue());
		}
		//get queried md
		for(String name : query){
			Node n=name2node.get(name);
			n.setCacheMD(null);
			res.put(name, n.getCacheMD());
		}
		return res;
	}
	/**
	 * Predict all the other nodes' states based on given states.
	 * 
	 * @param given	the map between given nodes and their states
	 * @return
	 */
	public Map<String,Integer> predict(Map<String,Integer> given){
		List<String> query=new ArrayList<String>();
		for(Node n : nodes){
			String name=n.getName();
			if(!given.containsKey(name))
				query.add(name);
		}
		return predict(given,query);
	}
	
	
	//Trivial getter & setter:
	public ArrayList<Node> getNodes() {
		return nodes;
	}
	public int getnNodes() {
		return nodes.size();
	}
	public FileSystem getHdfs() {
		return hdfs;
	}
		
}
