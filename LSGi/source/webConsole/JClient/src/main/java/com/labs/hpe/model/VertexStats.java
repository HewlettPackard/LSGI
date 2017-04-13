/**
 * 
 */
package com.labs.hpe.model;

import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Structure to manage vertex statistics 
 * @author Janneth Rivera, Tere Gonzalez
 * Feb 23, 2016
 */
public class VertexStats implements Serializable{
	public long nonconverge;
	public long iteration;
	public long time;
	public long totalVertices;
	public long state0;
	/**
	 * convergence
	 * This value is not being sent by QueryService, 
	 * but calculated in ClientSocket, so it must be IGNORED
	 * when reading bytes from socket.
	*/
	public double convergence; 
	
	
	public VertexStats(){
		
	}
	
	/**
	 * Read bytes from byte array
	 * @param byte[] 
	 */
	public void readBytes(byte[] bytes) {
		ByteBuffer buff = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);
		this.nonconverge = buff.getLong();
		this.iteration = buff.getLong();
		this.time = buff.getLong();
		this.totalVertices = buff.getLong();
		this.state0 = buff.getLong();
	}
	
	/** Get structure size in bytes
	 * @return structure size in bytes
	 */
	public static int sizeInBytes() {
		int bytes = (Long.SIZE + Long.SIZE + Long.SIZE + Long.SIZE + Long.SIZE)/8;	//bits to bytes
		//System.out.println("VertexStats size: "+ (bytes));
		return 	bytes;
	}
	
	
}
