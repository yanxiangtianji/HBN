package util;

import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;

public class HDFSWrite {

	public static void writeTimeMilli(FileSystem hdfs, long millis, Path path) throws IOException{
		PrintWriter pw=new PrintWriter(new OutputStreamWriter(hdfs.create(path,true)));
		pw.println(millis/1000+" seconds");
		pw.println(millis/1000/60+" minutes");
		pw.println(millis/1000/3600+" hours");
		pw.close();
	}

}
