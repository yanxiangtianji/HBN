package hbn;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Scanner;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;

public class BasicUpdater {
	float maxp;

	/**
	 * Configurate maxp used for each merge operation
	 * 
	 * @param maxp
	 */
	public BasicUpdater(float maxp) {
		this.maxp = maxp;
	}
	public BasicUpdater(double maxp) {
		this.maxp = (float)maxp;
	}
	/**
	 * Update the network's CDT, by merging the model in the incrementModelFile.
	 * Requirement: the model structure in incrementModelFile is the same to the
	 * network "net". Otherwise the behavior is undefined.
	 * 
	 * @param net
	 * @param incrementModelFile
	 * @param maxp
	 */
	public void incrementalUpdate(Network net, String incrementModelFile) {
		// Network netInc=new Network(this,false,false);
		Scanner scn = null;
		try {
			scn = new Scanner(new BufferedReader(
					new InputStreamReader(net.getHdfs().open(new Path(incrementModelFile)))));
			for (int i = 0; i < net.getnNodes(); i++) {
				String name = scn.next();
				int numParent = scn.nextInt();
				Node n = net.map2node(name);
				if (n == null || n.getParents().size() != numParent)
					throw new Exception("Basic node information does not match.");
				// marginal distribution:
				int nState=n.getnState();
				float[] md = new float[nState];
				for (int j = 0; j < numParent; j++)
					md[j] = scn.nextFloat();
				int sum = scn.nextInt();
				mergeDistribution(n.getDefaultMD(), n.getOccurrenceAll(), md, sum);
				n.setOccurrenceAll(n.getOccurrenceAll()+sum);
				// parents:
				if (numParent == 0) // no parent
					continue;
				for (int j = 0; j < numParent; j++) {	//check parents
					name = scn.next();
					if (n.getParent(j).getName() != name)
						throw new Exception("Node " + n.getName() + "'s " + j
								+ " parent information does not match (want:" + n.getParent(j) + "get:" + name + ")");
				}
				int nCDT=scn.nextInt();
				while(nCDT-->0){
					int offset=scn.nextInt();
					for(int j=0;j<nState;j++)
						md[j]=scn.nextFloat();
					sum=scn.nextInt();
					mergeDistribution(n.getCDT(offset),n.getOccurrenceCDT(offset),md,sum);
					n.setOccurrenceCDT(offset, n.getOccurrenceCDT(offset)+sum);
				}
			}
		} catch (IOException e) {
			System.err.println("Cannot open incremental model file.");
			e.printStackTrace();
		} catch (Exception e) {
			System.err.println("Logic error in updating model.");
			e.printStackTrace();
		} finally {
			scn.close();
		}
	}
	
	/**
	 * Incrementally update model of oldModelFile via incrementModelFile to newModelFile.
	 * 
	 * @param hdfs
	 * @param nodeFile
	 * @param knowledgeFile
	 * @param oldModelFile
	 * @param incrementModelFile
	 * @param newModelFile
	 * @throws Exception
	 */
	public void incrementalUpdateFile(FileSystem hdfs, String nodeFile, String knowledgeFile,
			String oldModelFile, String incrementModelFile, String newModelFile) throws Exception{
		Network net=new Network(hdfs,nodeFile,knowledgeFile);
		net.loadStructure(oldModelFile, true, true);
		incrementalUpdate(net,incrementModelFile);
		net.outputStructure(false,newModelFile, true);
	}

	/**
	 * Merge dis2 to dis1.
	 * 
	 * @param dis1
	 * @param sum1	Number of occurrence related to dis1
	 * @param dis2
	 * @param sum2	Number of occurrence related to dis2
	 */
	public void mergeDistribution(float[] dis1, int sum1, float[] dis2, int sum2) {
		float p = Math.min((float) sum1 / sum2, maxp);
		float factor = 1.0f / (1 + p);
		for (int i = 0; i < dis1.length; i++) {
			dis1[i] = (p * dis1[i] + dis2[i]) * factor;
		}
	}

	public void mergeDistributionSafe(float[] dis1, int sum1, float[] dis2, int sum2) throws Exception {
		if (dis1.length == dis2.length && sum2 != 0)
			mergeDistribution(dis1, sum1, dis2, sum2);
		else {
			String problem = (dis1.length == dis2.length ? "" : " length of two distribution do not match("
					+ dis1.length + " and " + dis2.length + ").")
					+ (sum2 != 0 ? "" : " sum2 is 0.");
			throw new Exception("parameter is illegal for:" + problem);
		}
	}

	// trivial getter & setter
	public float getMaxp() {
		return maxp;
	}

}
