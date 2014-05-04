package util;

import java.util.Arrays;

public class StringConvert {
	
	public static int[] setArray(int length, String values) {
		int[] arr = new int[length];
		if (length != 0) {
			int p = 0;
			for (String s : values.split(",")) {
				arr[p++] = Integer.parseInt(s);
			}
		}
		Arrays.sort(arr);
		return arr;
	}
}
