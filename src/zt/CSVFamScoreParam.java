package zt;

import java.util.ArrayList;

public class CSVFamScoreParam implements Cloneable{
	private String csvConfFile, input, output;
	private ArrayList<Integer> given, possible;
	private int specified;
	
	public CSVFamScoreParam() {
		given=new ArrayList<Integer>();
		possible=new ArrayList<Integer>();
	}
	public CSVFamScoreParam(String csvConfFile, String input, String output, ArrayList<Integer> given,
			ArrayList<Integer> possible, int specified) {
		this.csvConfFile = csvConfFile;
		this.input = input;
		this.output = output;
		this.given = given;
		this.possible = possible;
	}
	public CSVFamScoreParam(String args) throws Exception{
		this(args.split(" "));
	}
	public CSVFamScoreParam(String[] args) throws Exception{
		int with_conf=0;
		for (int i = 0; i < args.length; ++i) {
			switch(args[i]){
			case "-csvconf":
				csvConfFile=args[++i];
				with_conf|=1;
				break;
			case "-given":
				given=new ArrayList<Integer>();
				for(String s : args[++i].split("\\D")){
					given.add(Integer.parseInt(s));
				}
				with_conf|=2;
				break;
			case "-specified":
				specified=Integer.parseInt(args[++i]);
				with_conf|=4;
				break;
			case "-possible":
				possible=new ArrayList<Integer>();
				for(String s : args[++i].split("\\D")){
					possible.add(Integer.parseInt(s));
				}
				with_conf|=8;
				break;
			case "-input":
				input=args[++i];
				with_conf|=16;
				break;
			case "-output":
				output=args[++i];
				with_conf|=32;
				break;
			default:
			}
		}
		if (with_conf!=64-1) {
			String str="No enough configuration! (error code="+with_conf+")";
			System.err.println(str);
			throw new Exception(str);
		}
	}
	
	public String[] toArgs(){
		String[] res=new String[12];
		res[0]="-csvconf";	res[1]=csvConfFile;
		res[2]="-given";	res[3]=given.toString().replaceAll("[\\[\\] ]", "");
		res[4]="-possible";	res[5]=possible.toString().replaceAll("[\\[\\] ]", "");
		res[6]="-specified";	res[7]=String.valueOf(specified);
		res[8]="-input";	res[9]=input;
		res[10]="-output";	res[11]=output;
		return res;
	}
	
	public void movePossible2Given(int p){
		removePossible(p);
		addGiven(p);
	}
	public void addGiven(int p){
		given.add(p);
	}
	public boolean removePossible(int p){
		return possible.remove(new Integer(p));
	}

	@SuppressWarnings("unchecked")
	@Override
	public CSVFamScoreParam clone() throws CloneNotSupportedException {
		CSVFamScoreParam other=new CSVFamScoreParam();
//		other=(CSVFamScoreParam)super.clone();
		other.csvConfFile=csvConfFile;
		other.input=input;
		other.output=output;
		other.specified=specified;
		other.given=(ArrayList<Integer>) given.clone();
		other.possible=(ArrayList<Integer>) possible.clone();
		return other;
	}
	
	//Getter & Setter:
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

	public ArrayList<Integer> getGiven() {
		return given;
	}

	public void setGiven(ArrayList<Integer> given) {
		this.given = given;
	}

	public ArrayList<Integer> getPossible() {
		return possible;
	}

	public void setPossible(ArrayList<Integer> possible) {
		this.possible = possible;
	}

	public int getSpecified() {
		return specified;
	}

	public void setSpecified(int specified) {
		this.specified = specified;
	}
}
