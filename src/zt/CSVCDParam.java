package zt;



public class CSVCDParam {
	private String csvConfFile, input, output;
	private String briefStructureForCSVFile;
	
	public CSVCDParam(){
	}
	public CSVCDParam(String csvConfFile, String input, String output, String briefStructureForCSVFile) {
		super();
		this.csvConfFile = csvConfFile;
		this.input = input;
		this.output = output;
		this.briefStructureForCSVFile = briefStructureForCSVFile;
	}
	public CSVCDParam(String args) throws Exception{
		this(args.split(" "));
	}
	public CSVCDParam(String[] args) throws Exception{
		int with_conf=0;
		for(int i=0;i<args.length;++i){
			switch(args[i]){
			case "-csvconf":
				csvConfFile=args[++i];
				with_conf|=1;
				break;
			case "-structure":
				briefStructureForCSVFile=args[++i];
				with_conf|=2;
				break;
			case "-input":
				input=args[++i];
				with_conf|=4;
				break;
			case "-output":
				output=args[++i];
				with_conf|=8;
				break;
			default:
			}
		}
		if(with_conf!=15){
			String str="No enough configuration! (error code="+with_conf+")";
			System.err.println(str);
			throw new Exception(str);
		}
	}
	
	public String[] toArgs(){
		String[] res=new String[8];
		res[0]="-csvconf";	res[1]=csvConfFile;
		res[2]="-structure";	res[3]=briefStructureForCSVFile;
		res[4]="-input";	res[5]=input;
		res[6]="-output";	res[7]=output;
		return res;
	}
	
	//getters & setters:
	public String getCsvConfFile() {
		return csvConfFile;
	}
	public void setCsvConfFile(String csvConfFile) {
		this.csvConfFile = csvConfFile;
	}
	public String getInput() {
		return input;
	}
	public void setInput(String input) {
		this.input = input;
	}
	public String getOutput() {
		return output;
	}
	public void setOutput(String output) {
		this.output = output;
	}
	public String getBriefStructureForCSVFile() {
		return briefStructureForCSVFile;
	}
	public void setBriefStructureForCSVFile(String briefStructureForCSVFile) {
		this.briefStructureForCSVFile = briefStructureForCSVFile;
	}

}
