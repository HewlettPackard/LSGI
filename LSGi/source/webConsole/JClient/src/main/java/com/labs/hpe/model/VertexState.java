/**
 * References: 
 * https://en.wikipedia.org/wiki/Data_structure_alignment
 * http://stackoverflow.com/questions/119123/why-isnt-sizeof-for-a-struct-equal-to-the-sum-of-sizeof-of-each-member
 */
package com.labs.hpe.model;

import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Structure to manage vertex state results
 * @author Janneth Rivera, Tere Gonzalez
 * Feb 23, 2016
 */
public class VertexState implements Serializable{
	public long     vid;
	public float	p_0;
	public char  	state;
	public int		iteration;
	
	
	
	public VertexState() {

	}
	
	/**
	 * Read bytes from byte array
	 * @param byte[] 
	 */
	public void readBytes(byte[] bytes) {
		ByteBuffer buff = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);
		this.vid = buff.getLong();
		this.p_0 = buff.getFloat();		
		this.state = (char)buff.get();
        buff.get();buff.get();buff.get(); // consume padding bytes
        this.iteration = buff.getInt();
	
		
	}
	
	/** Get structure size in bytes
	 * @see references	
	 * @return structure size in bytes
	 */
	public static int sizeInBytes() {		
		//[8|4|1|3(pad)|4|4(pad)]= 24
        int bytes = (Long.SIZE + Float.SIZE + Byte.SIZE + Integer.SIZE)/8;      //bits to bytes
        int cPadding = 7;       //C/C++ padding for data alignment
        //System.out.println("VertexState size: "+ (bytes + cPadding));

        return (bytes + cPadding);
	}
	
	
}
