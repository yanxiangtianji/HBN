package application;

import hbn.BasicUpdater;
import hbn.Network;

import java.util.HashMap;
import java.util.Map.Entry;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;

public class IncrementalUpdate {

	public static void main(String[] args) throws Exception{
		System.out.println("Incremental Updating:");
		
		FileSystem hdfs=FileSystem.get(new Configuration());
		HashMap<String,String> properties=new HashMap<String,String>();
		String PREFIX="traffic3/";
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
		properties.put("incrStructureFile",PREFIX+"structure_ince.txt");
		properties.put("newStructureFile",PREFIX+"structure_new.txt");
		//for local debug
		for(Entry<String,String> entry: properties.entrySet())
			entry.setValue("../tmp/"+entry.getValue());
		//run:
		Network net=new Network(hdfs,properties.get("nodeFile"), properties.get("knowledgeFile"));
		net.loadStructure(properties.get("structureFile"), true, true);
		
		BasicUpdater updater=new BasicUpdater(3.0f);
		updater.incrementalUpdate(net, properties.get("incrStructureFile"));
		net.outputStructure(properties.get("newStructureFile"), true);
		
		System.out.println("Finished.");
	}

}
