/**
 * 
 */
package com.labs.hpe.service;

//import org.apache.log4j.Logger;

import com.labs.hpe.model.QueryRequest;

/**
 * This client will connect a client to request 
 * the type of query over the inference.
 * @author Janneth Rivera, Tere Gonzalez
 * Feb 23, 2016
 */
public class QueryClient {

	//private static final Logger logger = Logger.getLogger(QueryClient.class);
	
	private String serverAddress="localhost";//"192.168.122.172"; 
	private int serverPort=58000;
	
	public void runQueryClient(QueryRequest request){
		try{
			System.out.println("-- Getting inference state: connecting to " + serverAddress + ":" + serverPort + "--");
			ClientSocket cs = new ClientSocket();
			//cs.connectToServer(serverPort, serverAddress, request);
			cs.connect(serverPort, serverAddress);
			cs.send(request);
			cs.receive();			
			cs.close();
		}catch(Exception e){        	
        	//logger.error(e);
			e.printStackTrace();
        }
	}
	


	/**
	 * @return the serverAddress
	 */
	public String getServerAddress() {
		return serverAddress;
	}



	/**
	 * @param serverAddress the serverAddress to set
	 */
	public void setServerAddress(String serverAddress) {
		this.serverAddress = serverAddress;
	}



	/**
	 * @return the serverPort
	 */
	public int getServerPort() {
		return serverPort;
	}



	/**
	 * @param serverPort the serverPort to set
	 */
	public void setServerPort(int serverPort) {
		this.serverPort = serverPort;
	}



	/**
	 * Main Driver for the query service client
	 * This client will connect a client to request
	 * the type of query over the inference.
	 *
	 * Will have 3 types of request:
	 *
	 * 1.search state of a given vertex id
	 * 2.retrieve the state of all the vertices
	 * 3.Retrieve the state of a vertices that P(v)>=threshold.
	 *
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		try{
			if(args.length < 4){
				System.out.println("Incorrect Usage: <type of query=1 (byVId), =3 (byThreshold3), =5 (byTresholdStats) <value> <server =default localhost> <port =default 58000> )");
				System.out.println("Example: ./clientService 1 0 -> for query type 1 ");
				return;
			}
			
				
			QueryRequest request = new QueryRequest();
			request.type = Integer.parseInt(args[0]);
			request.value = Integer.parseInt(args[1]);		
			
			/*send query to inference service*/		
			QueryClient qc = new QueryClient();
			qc.setServerAddress(args[2]);
			qc.setServerPort(Integer.parseInt(args[3]));
			qc.runQueryClient(request);
			
		}catch(Exception e){
        	//logger.error(e);
        }
	}

}
