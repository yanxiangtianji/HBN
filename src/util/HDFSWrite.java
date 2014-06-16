package util;

import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.text.DecimalFormat;
import java.text.NumberFormat;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;

public class HDFSWrite {
	private static DecimalFormat df=(DecimalFormat)NumberFormat.getInstance();

	public static void writeTimeMilli(FileSystem hdfs, long millis, Path path) throws IOException{
		df.setMaximumFractionDigits(2);
		df.setMinimumFractionDigits(2);
		PrintWriter pw=new PrintWriter(new OutputStreamWriter(hdfs.create(path,true)));
		pw.println(millis/1000+" seconds");
		pw.println(df.format((double)millis/1000/60)+" minutes");
		pw.println(df.format((double)millis/1000/3600)+" hours");
		pw.close();
	}

}
