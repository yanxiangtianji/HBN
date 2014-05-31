package application;

import hbn.Network;

import java.util.HashMap;
import java.util.Map;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;

import util.ArgsParser;
import util.HDFSWrite;

public class Train {
	

	public static void main(String[] args) throws Exception{
		System.out.println("Train:");
		
//		String PREFIX="../tmp/traffic3/";
//		String PREFIX="traffic3/";
		Map<String,String> argsMap=ArgsParser.parse(args);
		String PREFIX=argsMap.get("-prefix");
		if(!PREFIX.endsWith("/"))
			PREFIX=PREFIX+"/";
		double improvement=500.0;
		if(argsMap.get("-improvement")!=null)
			improvement=Double.parseDouble(argsMap.get("-improvement"));	//throws when argument is wrong
		if(improvement<=0)
			throw new Exception("Improvement value is illegal: "+improvement);
		int multiThread=1;	//1,0 -> single thread
		if(argsMap.get("-mt")!=null)
			multiThread=Integer.parseInt(argsMap.get("-mt"));
		if(multiThread<0)
			throw new Exception("Number of thread is illegal: "+multiThread);
		
		String modelFolder=null;
		if(multiThread>=2)
			modelFolder="p"+(int)improvement+"mt/";
		else
			modelFolder="p"+(int)improvement+"/";
		
		System.out.println("prefix: "+PREFIX);
		System.out.println("multi-thread: "+multiThread);
		System.out.println("improvement: "+improvement);
		
		HashMap<String,String> properties=new HashMap<String,String>();
		properties.put("nodeFile", PREFIX+"conf/node.txt");
		properties.put("knowledgeFile", PREFIX+"conf/knowledge.txt");
		properties.put("csvHeadFile", PREFIX+"conf/csvhead.csv");
		properties.put("csvConfFile", PREFIX+"conf/csvconf.txt");
		properties.put("csvFolder", PREFIX+"input/");
		properties.put("famScoreFolder", PREFIX+"output_famscore/");
		properties.put("structureBriefFile", PREFIX+modelFolder+"structure_brief.txt");
		properties.put("structureCSVBriefFile", PREFIX+modelFolder+"structure_brief_csv.txt");
		properties.put("structureFile",PREFIX+modelFolder+"structure.txt");
		properties.put("distributionFolder", PREFIX+modelFolder+"output_distribution/");

		//run:
		long time=System.currentTimeMillis();
		
		FileSystem hdfs=FileSystem.get(new Configuration());
		Network net=new Network(hdfs,properties.get("nodeFile"), properties.get("knowledgeFile"));
		net.setCSVFormat(properties.get("csvHeadFile"),properties.get("csvConfFile"));
		if(multiThread>=2){
			net.greedyLearningMT(properties.get("csvFolder"),properties.get("csvConfFile"),
					properties.get("famScoreFolder"),improvement,multiThread);
		}else{
			net.greedyLearning(properties.get("csvFolder"),properties.get("csvConfFile"),
					properties.get("famScoreFolder"),improvement);
		}
//		net.loadStructure(properties.get("structureBriefFile"), false, false);
//		net.loadStructure(properties.get("structureFile"), true, true);
		
		net.outputBriefStructureWithName(properties.get("structureBriefFile"));
		net.outputBriefStructureWithCSVoff(properties.get("structureCSVBriefFile"));
//		net.calDistribution(properties.get("csvFolder"), properties.get("structureCSVBriefFile"),
//				properties.get("csvConfFile"), properties.get("distributionFolder"));
//		net.outputStructure(properties.get("structureFile"),true);
				
		time=System.currentTimeMillis()-time;
		HDFSWrite.writeTimeMilli(hdfs,time,new Path(PREFIX+modelFolder+"time.txt"));
		System.out.println("Finished.");
	}
	

}
