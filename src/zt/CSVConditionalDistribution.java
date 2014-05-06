/*
 * Get the DISTRIBUTION of each possible structure, given existing parents' ID, child's ID 
 * and possible parents ID by JobConf. 
 * Mapper output: (ParID, code of parents' values)->(child value, times)
 * Reducer output: key->parentID; value-> Score
 */
package zt;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.Scanner;
import java.util.TreeMap;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.filecache.DistributedCache;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.TextInputFormat;
import org.apache.hadoop.mapred.TextOutputFormat;
import org.apache.hadoop.util.StringUtils;

import util.StringConvert;
import Entity.PairIntWritable;
import Entity.VectorIntWritable;

public class CSVConditionalDistribution {
	
	// Mapper
	public static class CSVMapper extends MapReduceBase implements
			Mapper<LongWritable, Text, PairIntWritable, PairIntWritable> {
		private CSVStructure csv_structure;
		private HashMap<Integer,int[]> net_structure;
		
		@Override
		public void configure(JobConf job) {
			super.configure(job);
			loadCSVStructure(job);
			try {
				Path[] path=DistributedCache.getLocalCacheFiles(job);
				loadNetStructure(job,path[0]);
			}catch(FileNotFoundException e){
				System.err.println("Caught open structure file: "
						+ StringUtils.stringifyException(e));
				System.exit(0);
			}catch (IOException e) {
				System.err.println("Caught exception while getting cached files: "
						+ StringUtils.stringifyException(e));
				System.exit(0);
			}
		}
		private void loadCSVStructure(JobConf job){
			int[] D=StringConvert.setArray(job.getInt("csvcd.csv.num", -1),job.get("csvcd.csv.d"));
			csv_structure=new CSVStructure(D,null);		
		}
		private void loadNetStructure(JobConf job, Path structureFile) throws FileNotFoundException{
			Scanner scn = new Scanner(new BufferedReader(new FileReader(structureFile.toString())));
			int n=job.getInt("csvcd.csv.num", -1);
			net_structure=new HashMap<Integer,int[]>();
			for(int i=0;i<n;i++){
				int key=scn.nextInt();
				int num=scn.nextInt();
				int[] parents=null;
				if(num!=0){//we want to calculate marginal distribution of those non-parent nodes via this job.
					parents=new int[num];
					for(int j=0;j<num;j++)
						parents[j]=scn.nextInt();
				}
				net_structure.put(key, parents);
			}
			scn.close();
		}

		@Override
		public void map(LongWritable key, Text value, OutputCollector<PairIntWritable, PairIntWritable> output,
				Reporter reporter) throws IOException {
			String[] strs = value.toString().split(",");
			int[] nums=new int[strs.length];
			for(int i=0;i<strs.length;i++)
				nums[i]=Integer.parseInt(strs[i]);
			int oParents,oValue;
			for(Entry<Integer,int[]> entry : net_structure.entrySet()){
				oValue=nums[entry.getKey()];
				if(oValue==CSVCommon.INVALID_STATE ){
					continue;
				}else if(entry.getValue()==null){
					//non-parent nodes -> map all to 0 for marginal distribution 
					output.collect(new PairIntWritable(entry.getKey().intValue(),0), new PairIntWritable(oValue,1));
				}else if((oParents=csv_structure.encode(entry.getValue(), nums))!=CSVCommon.INVALID_STATE){
					//normal nodes
					output.collect(new PairIntWritable(entry.getKey().intValue(),oParents),
							new PairIntWritable(oValue,1));
				}
			}
		}
	}

	// Combiner:
	public static class CSVCombiner extends MapReduceBase implements
			Reducer<PairIntWritable, PairIntWritable, PairIntWritable, PairIntWritable> {
//		private CSVStructure csv_structure;

		@Override
		public void configure(JobConf job) {
			super.configure(job);
			loadCSVStructure(job);
		}
		private void loadCSVStructure(JobConf job){
//			int[] D=StringConvert.setArray(job.getInt("csvcd.csv.num", -1),job.get("csvcd.csv.d"));
//			csv_structure=new CSVStructure(D,null);		
		}

		@Override
		public void reduce(PairIntWritable key, Iterator<PairIntWritable> values,
				OutputCollector<PairIntWritable, PairIntWritable> output, Reporter reporter) throws IOException {
			TreeMap<Integer,Integer> map=new TreeMap<Integer,Integer>();
			while (values.hasNext()) {
				PairIntWritable p = values.next();
//				map.put(p.getV1(),map.get(p.getV1())+p.getV2());
				Integer v=map.get(p.getV1());
				if(v==null)
					map.put(p.getV1(),1);
				else
					map.put(p.getV1(), v+1);
			}
			for (Entry<Integer,Integer> entry: map.entrySet()) {
				output.collect(key, new PairIntWritable(entry.getKey(),entry.getValue()));
			}
		}
	}

	// Reducer:
	public static class ValReducer extends MapReduceBase implements
			Reducer<PairIntWritable, PairIntWritable, PairIntWritable, VectorIntWritable> {
		private CSVStructure csv_structure;

		@Override
		public void configure(JobConf job) {
			super.configure(job);
			loadCSVStructure(job);
		}
		private void loadCSVStructure(JobConf job){
			int[] D=StringConvert.setArray(job.getInt("csvcd.csv.num", -1),job.get("csvcd.csv.d"));
			csv_structure=new CSVStructure(D,null);		
		}

		@Override
		public void reduce(PairIntWritable key, Iterator<PairIntWritable> values,
				OutputCollector<PairIntWritable, VectorIntWritable> output, Reporter reporter) throws IOException {
			int size=csv_structure.getD(key.getV1());
			int[] res = new int[size];
			while (values.hasNext()) {
				PairIntWritable p = values.next();
				// System.out.println(v);
				res[p.getV1()] += p.getV2();
			}
			output.collect(key, new VectorIntWritable(res));
		}
	}
	 
	// Main Class:
	private CSVCDParam param;
	
//	public CSVConditionalDistribution() {}
	public CSVConditionalDistribution(CSVCDParam param){
		configure(param);
	}	
	public void configure(CSVCDParam param){
		this.param=param;
	}
	private void setConfHBN(JobConf conf) throws IOException {
		FileSystem hdfs = FileSystem.get(conf);
		Scanner s = new Scanner(hdfs.open(new Path(param.getCsvConfFile())));
		int N = s.nextInt();
		ArrayList<Integer> D = new ArrayList<Integer>();
		for (int i = 0; i < N; i++) {
			D.add(s.nextInt());
		}
		s.close();
		conf.setInt("csvcd.csv.num", N);
		conf.set("csvcd.csv.d", D.toString().replaceAll("[\\[\\] ]", ""));
		DistributedCache.addCacheFile(new Path(param.getBriefStructureForCSVFile()).toUri(), conf);
	}
	
	private void setJobIOPath(JobConf conf){
		FileInputFormat.setInputPaths(conf, new Path(param.getInput()));
		FileOutputFormat.setOutputPath(conf, new Path(param.getOutput()));
	}

	public int run() throws Exception {
		Configuration baseConf=new Configuration(); 
		JobConf conf1 = new JobConf(baseConf, CSVConditionalDistribution.class);
		setConfHBN(conf1);
		setJobIOPath(conf1);
		conf1.setJobName("cdistribution");
		
		conf1.setOutputKeyClass(PairIntWritable.class);
		conf1.setOutputValueClass(VectorIntWritable.class);
		conf1.setMapOutputKeyClass(PairIntWritable.class);
		conf1.setMapOutputValueClass(PairIntWritable.class);
		
		conf1.setMapperClass(CSVMapper.class);
		conf1.setCombinerClass(CSVCombiner.class);
		conf1.setReducerClass(ValReducer.class);

		conf1.setInputFormat(TextInputFormat.class);
		conf1.setOutputFormat(TextOutputFormat.class);

//		conf.setNumReduceTasks(2);
		
		JobClient.runJob(conf1);//launch 1st job:		
//		System.out.println("Job 1 finished!");
		
		return 0;
	}

	public static void main(String[] args) throws Exception {
		args="-csvconf ../tmp/conf.txt -structure ../tmp/brief_structure_csv.txt -input ../tmp/0.csv -output ../tmp/output8".split(" ");
		CSVCDParam param=new CSVCDParam(args);
		CSVConditionalDistribution f=new CSVConditionalDistribution(param);
		f.run();
	}
}
