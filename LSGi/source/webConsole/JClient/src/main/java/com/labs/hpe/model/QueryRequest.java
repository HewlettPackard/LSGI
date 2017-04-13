/**
 * 
 */
package com.labs.hpe.model;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;


/**
 * Structure to manage main request type
 * @author Janneth Rivera, Tere Gonzalez
 * Feb 23, 2016
 */
public class QueryRequest {
	public int type;
	public int value;
	
	public QueryRequest() {

	}
	
	
	/*public void readBytes(byte[] bytes) {
		ByteBuffer buff = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);
		this.x = buff.getInt();
		this.y = buff.getFloat();
		this.z = (char) buff.get();
	}*/

	public byte[] toByteArray() {
		ByteBuffer buff = ByteBuffer.allocate(QueryRequest.sizeInBytes()).order(ByteOrder.LITTLE_ENDIAN);
		buff.putInt(this.type);
		buff.putInt(this.value);
		return buff.array();
	}

	public static int sizeInBytes() {
		return Integer.SIZE + Integer.SIZE;
	}

	
}
