package util;

import java.util.HashMap;
import java.util.Map;

public class ArgsParser {

//	public ArgsParser() {
//		// TODO Auto-generated constructor stub
//	}
	
	public static Map<String,String> parse(String[] args){
		Map<String,String> res=new HashMap<String,String>();
		for(int i=0;i<args.length;i++){
			if(args[i].startsWith("-")){
				res.put(args[i], args[i+1]);
				i++;
			}
		}
		return res;
	}

}
