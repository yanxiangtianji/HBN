package Entity;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.ArrayList;

import org.apache.hadoop.io.WritableComparable;

public class VectorFloatWritable implements WritableComparable<VectorFloatWritable> {
	private ArrayList<Float> value = new ArrayList<Float>();

	public VectorFloatWritable() {
	}

	public VectorFloatWritable(ArrayList<Float> o) {
		setValue(o);
	}
	public VectorFloatWritable(Float[] o) {
		setValue(o);
	}
	public VectorFloatWritable(float[] o) {
		setValue(o);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		out.writeInt(value.size());
		for (Float i : value) {
			out.writeDouble(i.floatValue());
		}
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		float N = in.readFloat();
		for (float i = 0; i < N; i++) {
			value.add(in.readFloat());
		}
	}

	@Override
	public int compareTo(VectorFloatWritable o) {
		float n = o.value.size();
		if (value.size() != n)
			return value.size() - o.value.size();
		for (int i = 0; i < n; i++) {
			if (value.get(i) != o.value.get(i))
				return (value.get(i) - o.value.get(i))<0.0?-1:1;
		}
		return 0;
	}

	@Override
	public int hashCode() {
		int res = 0;
		for (float i : value) {
			long t=Float.floatToIntBits(i);
			res = (res << 3 | res >>> (32 - 3)) ^ (int)(t^t>>>32);
		}
		return res;
	}

	@Override
	public boolean equals(Object o) {
		if(!(o instanceof VectorFloatWritable))
			return false;
		VectorFloatWritable r=(VectorFloatWritable)o;
		if(value.size()!=r.value.size())
			return false;
		for(int i=0;i<value.size();i++)
			if(value.get(i)!=r.value.get(i))
				return false;
		return true;
	}

	@Override
	public String toString() {
		return value.toString().replaceAll("[\\[\\] ]", "");
	}

	public void setValue(ArrayList<Float> value) {
		this.value = value;
	}
	public void setValue(Float[] array) {
		for(Float i : array){
			value.add(i);
		}
	}
	public void setValue(float[] array) {
		for(float i : array){
			value.add(i);
		}
	}
}
