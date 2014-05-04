package zt;

public class CSVStructure {
	private int[] D;
	private int[] given; // will be considered in each encode/deconde by default

	public CSVStructure(int[] dims, int[] given) {
		this.D = dims;
		this.given = given;
	}

	// encode
	public int encodeGiven(int[] l) {
		return encode(given, l);
	}

	public int encode(int[] caredIDX, int[] value) {
		int res = 0;
		for (int i : caredIDX) {
			int t = value[i];
			if (t == CSVCommon.INVALID_STATE)
				return -1;
			res = res * D[i] + t;
		}
		return res;
	}

	// decode
	public int[] decodeGiven(int code) {
		return decode(code, given);
	}

	public int[] decodeGivenAndPossible(int code, int possibleIDX) {
		int[] res = new int[given.length + 1];
		res[given.length] = code % D[possibleIDX];
		decode(code / D[possibleIDX], given, res);
		return res;
	}

	public int[] decode(int code, int[] caredIDX) {
		int res[] = new int[caredIDX.length];
		decode(code, caredIDX, res);
		return res;
	}

	private void decode(int code, int[] caredIDX, int[] res) {
		for (int i = caredIDX.length - 1; i >= 0; i--) {
			int limit = D[caredIDX[i]];
			res[i] = code % limit;
			code /= limit;
		}
	}

	// dimension
	public int[] getD() {
		return D;
	}

	public int getD(int offset) {
		return D[offset];
	}

	// given
	public int[] getGiven() {
		return given;
	}

	public void setGiven(int[] given) {
		this.given = given;
	}

}
