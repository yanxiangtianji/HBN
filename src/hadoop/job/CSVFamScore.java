/*
 * Get the FamScore of each possible structure, given existing parents' child's ID by JobConf. 
 * Mapper output: key->(parentID, value of parents); value->(value of child, times)
 * Reducer output: key->parentID; value-> Score
 */
package hadoop.job;

import hadoop.CSVCommon;
import hadoop.CSVFamScoreParam;
import hadoop.CSVStructure;

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Scanner;

import org.apache.commons.math3.special.Gamma;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.IntWritable;
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
import org.apache.hadoop.mapred.SequenceFileInputFormat;
import org.apache.hadoop.mapred.SequenceFileOutputFormat;
import org.apache.hadoop.mapred.TextInputFormat;
import org.apache.hadoop.mapred.TextOutputFormat;
import org.apache.hadoop.mapred.lib.IdentityMapper;

import util.StringConvert;
import Entity.PairIntWritable;

public class CSVFamScore {

	// Mapper
	public static class CSVMapper extends MapReduceBase implements
			Mapper<LongWritable, Text, PairIntWritable, PairIntWritable> {
		private int specified;
		private int[] possible;
		private CSVStructure structure;

		@Override
		public void configure(JobConf job) {
			super.configure(job);
			//csv configure
			int[] D=StringConvert.setArray(job.getInt("csvfamscore.csv.num", -1),job.get("csvfamscore.csv.d"));
			int[] given=StringConvert.setArray(job.getInt("csvfamscore.given.num", -1),job.get("csvfamscore.given.value"));
			Arrays.sort(given);
			structure=new CSVStructure(D,given);
			possible=StringConvert.setArray(job.getInt("csvfamscore.possible.num", -1),job.get("csvfamscore.possible.value"));
			Arrays.sort(possible);
			specified = job.getInt("csvfamscore.specified", -1);
		}

		@Override
		public void map(LongWritable key, Text value, OutputCollector<PairIntWritable, PairIntWritable> output,
				Reporter reporter) throws IOException {
			String[] strs = value.toString().split(",");
			int[] nums=new int[strs.length];
			for(int i=0;i<strs.length;i++)
				nums[i]=Integer.parseInt(strs[i]);
			int oBaseKey,oValue;
			if((oValue=nums[specified])==CSVCommon.INVALID_STATE
					|| (oBaseKey=structure.encodeGiven(nums))==CSVCommon.INVALID_STATE)
				return;	//get the basic parents' values code
			for(int offset : possible){
				//update parents' values code with "offset" at the end (oBaseKey*D[offset]+nums[offset])
				if(nums[offset]!=CSVCommon.INVALID_STATE)
					output.collect(new PairIntWritable(offset,oBaseKey*structure.getD(offset)+nums[offset]),
							new PairIntWritable(oValue,1));
			}
		}
	}

	// Combiner:
	public static class CSVCombiner extends MapReduceBase implements
			Reducer<PairIntWritable, PairIntWritable, PairIntWritable, PairIntWritable> {
		private int spfSize;

		@Override
		public void configure(JobConf job) {
			super.configure(job);
			int specified=job.getInt("csvfamscore.specified", -1);
			String str = job.get("csvfamscore.csv.d").split(",")[specified];
			spfSize = Integer.parseInt(str);
		}

		@Override
		public void reduce(PairIntWritable key, Iterator<PairIntWritable> values,
				OutputCollector<PairIntWritable, PairIntWritable> output, Reporter reporter) throws IOException {
			int[] res = new int[spfSize];
			while (values.hasNext()) {
				PairIntWritable p = values.next();
				// res[p.getV1()]+=p.getV2();
				res[p.getV1()]++;
			}
			for (int i = 0; i < spfSize; i++) {
				if (res[i] != 0)
					output.collect(key, new PairIntWritable(i, res[i]));
			}
		}
	}

	// Reducer:
	public static class ValReducer extends MapReduceBase implements
			Reducer<PairIntWritable, PairIntWritable, IntWritable, DoubleWritable> {
		private int spfSize;

		@Override
		public void configure(JobConf job) {
			super.configure(job);
			String str = job.get("csvfamscore.csv.d");
			List<Integer> D = new ArrayList<Integer>();
			for (String s : str.split(",")) {
				D.add(Integer.parseInt(s));
			}
			spfSize = D.get(job.getInt("csvfamscore.specified", -1));
		}

		@Override
		public void reduce(PairIntWritable key, Iterator<PairIntWritable> values,
				OutputCollector<IntWritable, DoubleWritable> output, Reporter reporter) throws IOException {
			int[] res = new int[spfSize];
			int sum=0;
			while (values.hasNext()) {
				PairIntWritable p = values.next();
				// System.out.println(v);
				res[p.getV1()] += p.getV2();
				sum += p.getV2();
			}
			double score = Gamma.logGamma(1.0 + sum);
			for (int i : res) {
				if(i==0)
					continue;
				score += Gamma.logGamma(1.0 + i);
			}
			output.collect(new IntWritable(key.getV1()), new DoubleWritable(score));
		}
	}
	
	public static class FamReducer extends MapReduceBase 
		implements Reducer<IntWritable,DoubleWritable,IntWritable,DoubleWritable>{
		@Override
		public void reduce(IntWritable key, Iterator<DoubleWritable> values,
				OutputCollector<IntWritable, DoubleWritable> output, Reporter reporter) throws IOException {
			double res=0.0;
			while(values.hasNext()){
				res+=values.next().get();
			}
			output.collect(key, new DoubleWritable(res));
		}		
	}
 
	// Main Class:
	private String tempPath=null;
	private CSVFamScoreParam param;
	
	public CSVFamScore() {}
	public CSVFamScore(CSVFamScoreParam param){
		configure(param);
	}	
	public void configure(CSVFamScoreParam param){
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
		conf.setInt("csvfamscore.csv.num", N);
		conf.set("csvfamscore.csv.d", D.toString().replaceAll("[\\[\\] ]", ""));
		conf.setInt("csvfamscore.given.num", param.getGiven().size());
		conf.set("csvfamscore.given.value", param.getGiven().toString().replaceAll("[\\[\\] ]", ""));
		conf.setInt("csvfamscore.possible.num", param.getPossible().size());
		conf.set("csvfamscore.possible.value", param.getPossible().toString().replaceAll("[\\[\\] ]",""));
		conf.setInt("csvfamscore.specified", param.getSpecified());
	}
	
	private void set2JobIOPath(JobConf conf1,JobConf conf2){
		int rand=Math.abs(new Random().nextInt());
//		tempPath="TEMP_FAMSCORE_"+System.currentTimeMillis()+"_"+rand;
		SimpleDateFormat formatter = new SimpleDateFormat("yyyyMMdd-HHmmss");
		String timeString = formatter.format(System.currentTimeMillis());
		tempPath=param.getInput().replaceAll("[^/]+$", "TEMP_FAMSCORE_"+timeString+"_"+rand);
		Path p=new Path(tempPath);
		//job1
		FileInputFormat.setInputPaths(conf1, new Path(param.getInput()));
		FileOutputFormat.setOutputPath(conf1, p);
		//job2
		FileInputFormat.setInputPaths(conf2, p);
		FileOutputFormat.setOutputPath(conf2, new Path(param.getOutput()));
	}
	private void set2JobName(JobConf conf1,JobConf conf2){
		int p=param.getSpecified();
		conf1.setJobName("famscore_front_"+p);
		conf2.setJobName("famscore_back_"+p);
	}
	private void CleanUp(JobConf conf) throws IOException{
		FileSystem hdfs=FileSystem.get(conf);
		hdfs.delete(new Path(tempPath),true);
	}

	public int run() throws Exception {
		Configuration baseConf=new Configuration(); 
		JobConf conf1 = new JobConf(baseConf, CSVFamScore.class);
		JobConf conf2 = new JobConf(baseConf, CSVFamScore.class);
		setConfHBN(conf1);
		set2JobIOPath(conf1,conf2);
		set2JobName(conf1,conf2);
		
		conf1.setOutputKeyClass(IntWritable.class);
		conf1.setOutputValueClass(DoubleWritable.class);
		conf1.setMapOutputKeyClass(PairIntWritable.class);
		conf1.setMapOutputValueClass(PairIntWritable.class);
		conf2.setOutputKeyClass(IntWritable.class);
		conf2.setOutputValueClass(DoubleWritable.class);
		
		conf1.setMapperClass(CSVMapper.class);
		conf1.setCombinerClass(CSVCombiner.class);
		conf1.setReducerClass(ValReducer.class);
		conf2.setMapperClass(IdentityMapper.class);
		conf2.setReducerClass(FamReducer.class);

		conf1.setInputFormat(TextInputFormat.class);
		conf1.setOutputFormat(SequenceFileOutputFormat.class);
		conf2.setInputFormat(SequenceFileInputFormat.class);
		conf2.setOutputFormat(TextOutputFormat.class);

//		conf.setNumReduceTasks(2);
		
			
		JobClient.runJob(conf1);//launch 1st job:		
//		System.out.println("Job 1 finished!");
		JobClient.runJob(conf2);//launch 2nd job:
		
		CleanUp(conf1);//clean up intermediate files	
		return 0;
	}

	public static void main(String[] args) throws Exception {
		args="-csvconf ../tmp/conf.txt -given 1,4 -possible 2,5,6 -specified 3 -input ../tmp/0.csv -output ../tmp/output7".split(" ");
		CSVFamScoreParam param=new CSVFamScoreParam(args);
		CSVFamScore f=new CSVFamScore(param);
		f.run();
	}
}
