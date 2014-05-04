package zt;


public class CSVStructure {
	private int[] D;
	private int[] given;
	
	public CSVStructure(int[] dims,int[] given){
		this.D=dims;
		this.given=given;
	}

	//encode
	public int encodeGiven(int[] l) {
		int res = 0;
		for (int i : given) {
			int t = l[i];
			if (t == CSVConditionalDistribution.getINVALID_STATE())
				return -1;
			res = res * D[i] + t;
		}
		return res;
	}

	//decode
	public int[] decodeGiven(int code) {
		int[] res = new int[given.length];
		decodeGiven(code, res);
		return res;
	}

	public int[] decodeAll(int code, int possibleIDX) {
		int[] res = new int[given.length + 1];
		res[given.length] = code % D[possibleIDX];
		decodeGiven(code / D[possibleIDX], res);
		return res;
	}

	private void decodeGiven(int code, int[] res) {
		for (int i = given.length - 1; i >= 0; i--) {
			int limit = D[given[i]];
			res[i] = code % limit;
			code /= limit;
		}
	}

	//dimension
	public int[] getD(){
		return D;
	}
	public int getD(int offset){
		return D[offset];
	}
}
