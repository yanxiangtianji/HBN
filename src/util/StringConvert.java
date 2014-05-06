package util;


public class StringConvert {
	
	/**
	 * generate an integer array with given length and content string separated with "," 
	 * 
	 * @param length
	 * @param values
	 * @return
	 */
	public static int[] setArray(int length, String values) {
		return setArray(length,values,",");
	}
	
	/**
	 * generate an integer array with given length and content string separated with given sepor 
	 * 
	 * @param length
	 * @param values
	 * @param sepor
	 * @return
	 */
	public static int[] setArray(int length, String values, String sepor) {
		int[] arr = new int[length];
		if (length != 0) {
			int p = 0;
			for (String s : values.split(sepor)) {
				arr[p++] = Integer.parseInt(s);
			}
		}
		return arr;
	}	
}
