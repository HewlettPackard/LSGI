/**
 * 
 */
package com.labs.hpe.service;



import java.io.OutputStream;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.HashMap;
import java.io.InputStream;

import org.apache.commons.lang3.SerializationUtils;
import org.apache.log4j.Logger;

import com.labs.hpe.model.QueryRequest;
import com.labs.hpe.model.VertexState;
import com.labs.hpe.model.VertexStats;


/**
 * This class contains methods to communicate the webservices with the QueryService.
 * This client connects a socket to the server (QueryService) 
 * and retrieve inference results.
 * @author Janneth Rivera, Tere Gonzalez
 * Feb 23, 2016
 */
public class ClientSocket {
	private static final Logger logger = Logger.getLogger(ClientSocket.class);
	
	private Socket clientSocket;
	private OutputStream out;
	private InputStream in;

	private HashMap<String,Object> responseObj;
	
	/**
	 * Method to use TCP socket to connect to
	 * the query service
	 *
	 */
	public int connectToServer(int serverPort, String serverAddress, QueryRequest request){
		try{					
			
			// connect
			clientSocket = new Socket(serverAddress,serverPort); 
			out = clientSocket.getOutputStream();
			in = clientSocket.getInputStream();

			switch(request.type) {
			case 1:
				System.out.println(" Query Type: [ Search by Vertex Id ]");
				System.out.println(" vertex#:    [ " + request.value + " ]");
				break;
			case 3: 
				System.out.println(" Query Type:  [ Search by Probability Threshold ]");
				System.out.println(" Probability: [ " + request.value + " ]");		
				break;
			default: System.out.println("Invalid Id"); return 1;
			}
									
			// send request		
			byte[] reqData = request.toByteArray();
			out.write(reqData);
			
			doQuery(in, request.type);
			
			clientSocket.close();     
		}catch(Exception e){  			
        	logger.error("Error while connecting to server. " + e);
        	e.printStackTrace();
        }
		return 0;			
	}
	
	
	/**
	 * Main query driver
	 * depending on the query type
	 */
	public void doQuery(InputStream in, int queryType){
		try{
			switch(queryType) {
			case 1: /*vid type*/
				doQueryByVid(in);
				break;
			case 2: /**/
				break;
			case 3:
				doQueryByThreshold(in);
				break;	
			case 4:
				break;
			case 5:
				/*Same as case#3, but only read stats*/
				doQueryThresholdStats(in); 
				break;
			default:
				System.out.println(" Invalid Query Type");
				return;
			}
		}catch(Exception e){  			
        	logger.error("Error while doQuery. " +e);
        }	
	}
	
	/**
	 * search specific vertex id
	 */
	public void doQueryByVid(InputStream in){
		try{
			responseObj = new HashMap<String,Object>();
			VertexState stateResponse = new VertexState();
			VertexStats statsResponse = new VertexStats();			
			
				
			// listen response
			byte[] stateData = new byte[VertexState.sizeInBytes()];
			in.read(stateData);
			stateResponse.readBytes(stateData);				
						
			//System.out.println(in.skip(4));
			
			// listen response
			byte[] statsData = new byte[VertexStats.sizeInBytes()];
			in.read(statsData);
			statsResponse.readBytes(statsData);
			
			
			System.out.println("\nInference Results");
            System.out.println("-------------------------------------------");
            System.out.println("vertex#\tstate \tprob_0 \titeration#");
            System.out.println("-------------------------------------------");

            System.out.println(stateResponse.vid + "\t" + stateResponse.state + "\t" + stateResponse.p_0 + "\t" + stateResponse.iteration);

            System.out.println("-------------------------------------------");
            System.out.println("Elapsed runtime(sec):            [ " + statsResponse.time + " ]");
            System.out.println("Non-converged vertices:          [ " + statsResponse.nonconverge + " ]");

            if (statsResponse.totalVertices > 0){
                long convergence = (long) (100.0 - (statsResponse.nonconverge * 100.0 / statsResponse.totalVertices));    
            	System.out.println("Convergence:                     [ " + convergence + " ]");
            	statsResponse.convergence = convergence;                    
            }
            
            System.out.println("State0:          [ " + statsResponse.state0 + " ]");
            
            responseObj.put("vId", stateResponse);
            responseObj.put("stats", statsResponse); 
			
		}catch(Exception e){  			
        	logger.error("Error while doQueryById. " +e);
        	e.printStackTrace();
        	 
        }	
	}

	
	/**
	 * Search a subset to be greater or equal to
	 * a threshold
	 */
	public void doQueryByThreshold(InputStream in){
		try{
			responseObj = new HashMap<String,Object>();
			VertexStats statsResponse = new VertexStats();
			VertexState stateResponse;

			/* count values */
			long counter = 0;
			long maxVal = 5;
			
			
			byte[] statsData = new byte[VertexStats.sizeInBytes()];
			in.read(statsData); 
			statsResponse.readBytes(statsData);
			

			byte[] counterData = new byte[(Long.SIZE/8)];
			in.read(counterData);
			counter = ByteBuffer.wrap(counterData).order(ByteOrder.LITTLE_ENDIAN).getLong();

			if (counter == 0) {
				System.out.println("\nNo elements found...\n");
				//return;
			}

			ArrayList<VertexState> vidResult = new ArrayList<VertexState>();
			byte[] stateData = new byte[VertexState.sizeInBytes()];
			int bytesRead = 0;
			for (int j = 0; j <  Math.min(maxVal, counter); j++) {
				bytesRead += in.read(stateData);
				stateResponse = new VertexState();
				stateResponse.readBytes(stateData);
				vidResult.add(stateResponse);
			}

			/* print first 5 */
			 System.out.println("\nInference Results");
             System.out.println("-------------------------------------------");
             System.out.println("vertex#\tstate \tprob_0 \titeration#");
             System.out.println("-------------------------------------------");

             for (int i = 0; i < Math.min(maxVal, counter); i++) {
                     System.out.println(vidResult.get(i).vid + "\t" + vidResult.get(i).state + "\t" + vidResult.get(i).p_0 + "\t" + vidResult.get(i).iteration);
             }

             System.out.println("Elements found >= the threshold: [ " + counter      + " ]");
             System.out.println("Listed First:                    [ 5 ]");
             System.out.println("-----------------------------------------");
             System.out.println("Elapsed runtime(sec):            [ " + statsResponse.time + " ]");
             System.out.println("Max Iteration:                   [ " + statsResponse.iteration + " ]");
             System.out.println("Non-converged vertices:          [ " + statsResponse.nonconverge + " ]");

             if (statsResponse.totalVertices > 0){
                 long convergence = (long) (100.0 - (statsResponse.nonconverge * 100.0 / statsResponse.totalVertices));    
             	System.out.println("Convergence:                     [ " + convergence + " ]");
             	statsResponse.convergence = convergence;                    
             }
             
             System.out.println("State0 vertices:          [ " + statsResponse.state0 + " ]");
             
             responseObj.put("topK", vidResult);
             responseObj.put("stats", statsResponse); 
             
		}catch(Exception e){  			
        	logger.error("Error while doQueryByThreshold. " +e);
        	e.printStackTrace();
        }
		
	}
	
	/**
	 * Search a subset to be greater or equal to
	 * a threshold. Only read stats.
	 */
	public void doQueryThresholdStats(InputStream in){
		try{
			responseObj = new HashMap<String,Object>();
			VertexStats statsResponse = new VertexStats();			
			
			byte[] statsData = new byte[VertexStats.sizeInBytes()];
			in.read(statsData); 
			statsResponse.readBytes(statsData);			

			System.out.println("Elements found >= the threshold: [ " + statsResponse.state0      + " ]");
			System.out.println("-----------------------------------------");
			System.out.println("Elapsed runtime(sec):            [ " + statsResponse.time + " ]");
			System.out.println("Max Iteration:                   [ " + statsResponse.iteration + " ]");
			System.out.println("Non-converged vertices:          [ " + statsResponse.nonconverge + " ]");

	         if (statsResponse.totalVertices > 0){ 
	             double convergence = (double) (100.0 - (statsResponse.nonconverge * 100.0 / statsResponse.totalVertices));    
	         	System.out.println("Convergence:                     [ " + convergence + " ]");
	         	statsResponse.convergence = convergence;                    
	         }
             
             System.out.println("State0 vertices:          [ " + statsResponse.state0 + " ]");
             
             
             responseObj.put("stats", statsResponse); 
             
		}catch(Exception e){  			
        	logger.error("Error while doQueryByThresholdStats. " +e);
        	e.printStackTrace();
        }
		
	}
	
//********** WEBSERVICE **************//
	public void connect(int serverPort, String serverAddress){		
		try{
			// connect
			clientSocket = new Socket(serverAddress,serverPort); 
			out = clientSocket.getOutputStream();
			in = clientSocket.getInputStream();			       
		}catch(Exception e){
			logger.error("Error while connect. " +e);
		}		
	}
	
	public void send(QueryRequest request){			
		try{			
			// send request		
			byte[] reqData = request.toByteArray();
			out.write(reqData);
			out.flush();
			//out.close();
			doQuery(in, request.type);
		}catch(Exception e){
			logger.error("Error while send. " +e);
		}
	}
	
	public byte[] receive(){			
		byte[] reply = null;
		try{
			logger.info("receive");			
			reply = SerializationUtils.serialize(responseObj);
			logger.info("after receive");
		}catch(Exception e){
			logger.error("Error while receive. " +e);
		}
		return reply;
	}
	
	public void close(){		
		try{
			
			in.close();
			out.close();			
			clientSocket.close();  
			logger.info("close");		
		}catch(Exception e){
			logger.error("Error while close. " +e);
		}
	}
	
}
