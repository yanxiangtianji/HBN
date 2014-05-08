package application;

import hbn.Network;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;

public class Test {

	public static void main(String[] args) throws Exception{
		FileSystem hdfs=FileSystem.get(new Configuration());
		HashMap<String,String> properties=new HashMap<String,String>();
		String PREFIX="../tmp/traffic3/";
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
//		net.greedyLearning(properties.get("csvFolder"),properties.get("csvConfFile"),
//				properties.get("famScoreFolder"),500.0);
//		net.loadStructure(properties.get("structureBriefFile"), false, false);
		net.loadStructure(properties.get("structureFile"), true, true);
		
//		net.outputBriefStructureWithName(properties.get("structureBriefFile"));
//		net.outputBriefStructureWithCSVoff(properties.get("structureCSVBriefFile"));
//		net.calDistribution(properties.get("csvFolder"), properties.get("structureCSVBriefFile"),
//				properties.get("csvConfFile"), properties.get("distributionFolder"));
//		net.outputStructure(properties.get("structureFile"),true);
		
		HashMap<String,Integer> given=new HashMap<String,Integer>();
		given.put("A1", 1);
		given.put("A2", 3);
		given.put("A3", 0);
		Map<String,Integer> res=net.predict(given);
		for(Entry<String,Integer> entry : res.entrySet()){
			System.out.println(entry.getKey()+"\t"+entry.getValue());
		}
		
		System.out.println("Finished.");
	}
	

}
