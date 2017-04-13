/*LSGi
“© Copyright 2017  Hewlett Packard Enterprise Development LP

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”
*/
package com.labs.hpe.service.controller;



import java.io.InputStream;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Properties;


import javax.annotation.PostConstruct;

import org.apache.commons.lang3.SerializationUtils;
import org.apache.log4j.Logger;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.type.TypeReference;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.ResponseBody;

import com.jcraft.jsch.Session;
import com.labs.hpe.model.QueryRequest;
import com.labs.hpe.service.ClientSocket;
import com.labs.hpe.service.ssh.SSHConnector;






/**
 * This class contains web services for LSGiWebConsole
 * @class LSGiController
 * @author Janneth Rivera, Tere Gonzalez
 * March 3, 2016
 */

@Controller
@RequestMapping("/theMachine")
public class LSGiController {

	private static final Logger logger = Logger.getLogger(LSGiController.class);	
	
	private InputStream in;
	
	private String DEMO_SCRIPTS_PATH;
	private String LAUNCH_SCRIPT;
	private String STOP_SCRIPT;	
	private int TUNNEL_PORT;		
	private int SSH_DEFAULT_PORT=22; //Default SSH port
	
	@PostConstruct
	public void init() {
		logger.info("Init LSGi controller");	
		
		try{	
			//Load properties
			getLSGiProperties();
			
		}catch(Exception e){
			logger.error(e);
		}
		
	}
	
	/**
	 * Read LSGi properties from config files under /src/main/resources/properties
	 */
	public void getLSGiProperties() {	    	
		logger.info("Getting LSGi properties");							

		try {							
			//Read properties from file		
			Properties prop = new Properties();		
			String file = "properties/lsgi.properties";			
			in = getClass().getClassLoader().getResourceAsStream(file);
			prop.load(in);	
			
			DEMO_SCRIPTS_PATH = prop.getProperty("demoScriptsPath");
			LAUNCH_SCRIPT = prop.getProperty("launchScript");
			STOP_SCRIPT = prop.getProperty("stopScript");	
			TUNNEL_PORT = Integer.parseInt(prop.getProperty("tunnelPort")); 		
						
			in.close();			
			
			System.out.println("demoScriptsPath=" + DEMO_SCRIPTS_PATH);			
			System.out.println("launchScript=" + LAUNCH_SCRIPT);
			System.out.println("stopScript=" + STOP_SCRIPT);
			System.out.println("tunnelPort=" + TUNNEL_PORT);		
		
			logger.info("Succesfully read LSGi properties.");			
		}
		catch(Exception e){
			logger.error("Error while getting LSGi properties. ", e);
		}
		
		return;    	
	}

	
	/**
	 * Get execution configurations
	 * <br><br>	 
	 * <code>
	 * Example:<br>
	 * OUTPUT:<br>
	 * 	{
	 *		"nodes": [{"id": 0,"name": "1 node","value": "1","image": "Infrastructure.png","description": "The 1 nodes&#46;&#46;&#46; More details about the nodes&#46;"}],
	 *		"environments": [{"id": 2,"name": "Single Node-L4TM","hostname": "build-l4tm-2.u.labs.hpecorp.net","image": "Servers.png","lsgi_home": "/home/gomariat/graphs/code/LSGi","description": "The l4tm-selfhosted-2 is a single node (DL580) running L4TM and librarian self hosted version&#46;","availableDatasets": [0,1,2,3,4],"availableNodes": [0,1,2,3,4,5,6]}],
	 *		"datasets": [{"id": 0,"name": "1 Billion Vertices","value": "/data/inputGraphs/largeDataset/gV1000M.bin","image": "Big_data.png","pollingTime": 48000,"loadingTime": 4000,"description": "The 1 billion vertex graph with 3.3 billion edges&#46;&#46;&#46; More details about the dataset&#46;","graphlabTime": -1,"queryServicePort": 58000,"statistics": {"vertices": 1000000000,"edges": 3392337264,"s0": 429382,"s1": 278726,"labels": 708108}}]
	 *	}
	 * </code>
	 * @method getExecutionConfiguration
	 * @return {Object}	Execution configurations in JSON format
	*/
	@RequestMapping(value = "/getExecutionConfiguration", method = RequestMethod.GET, produces = "application/json")
	@ResponseBody
	public HashMap<String, Object> getExecutionConfiguration() {	
		logger.info("Running getExecutionConfiguration");		
	  	
		HashMap<String, Object> response = new HashMap<String, Object>();			
				
		try{					
			ObjectMapper mapper = new ObjectMapper();			
			TypeReference<List<HashMap<String,Object>>> typeRef = new TypeReference<List<HashMap<String,Object>>>() {};
			
			//Read environments 
			String environmentsFile = "properties/environments.json";
			in = getClass().getClassLoader().getResourceAsStream(environmentsFile);
		    List<HashMap<String,Object>> obj1 = mapper.readValue(in, typeRef);
		    //System.out.println(obj1); 
		    
		    //Read datasets 
		    String datasetFile = "properties/datasets.json";
		    in = getClass().getClassLoader().getResourceAsStream(datasetFile);
			List<HashMap<String,Object>> obj2 = mapper.readValue(in, typeRef);
			//System.out.println(obj2); 
		    
			//Read nodes
		    String nodesFile = "properties/nodes.json";
		    in = getClass().getClassLoader().getResourceAsStream(nodesFile);
			List<HashMap<String,Object>> obj3 = mapper.readValue(in, typeRef);
			//System.out.println(obj3);
			
			response.put("environments", obj1);
			response.put("datasets", obj2);
			response.put("nodes", obj3);
				    			
			in.close();
		}catch(Exception e){
			logger.error(e);
			e.printStackTrace();			
			response.put("status", "error");		
		}
		
		return response;
	}
	
	

	/**
	 * Launches Job and QueryService in specific environment&#46;
	 * Verifies if "Done&#33;" string was returned in order to notify success state to the UI&#46;
	 * It also makes the assumption that if the environment to be run is "localhost" (or 127.0.0.1) 
	 * then a tunneling process is needed to access the real destination environment 
	 * (for example, when accessing a virtual machine without public IP) 
	 * <br><br>	
	 * <code>
	 * Example:<br>
	 * INPUT:<br>
	 * 	{"environment": "build-l4tm-2.u.labs.hpecorp.net", "lsgi_home": "/home/gomariat/graphs/code/LSGi", "nodes": "1", dataset: "/data/inputGraphs/dns_graph.alchemy.factors.bin"}<br>
	 * OUTPUT:<br>
	 * 	{"status":"success"}
	 * </code>
	 * @param request	The request object containing: environment, lsgi_home, nodes, dataset in JSON format
	 * @method launch
	 * @return {Object}	Launch results in JSON format
	*/
	@RequestMapping(value = "/launch", method = RequestMethod.POST, consumes = "application/json", produces = "application/json")
	@ResponseBody
	public HashMap<String, Object> launch(@RequestBody HashMap<String, Object> request) {		
		logger.info("Running launch");
		
		String serverAddress = (String) request.get("environment");
		String lsgi_home = (String) request.get("lsgi_home");
		String nodes = (String) request.get("nodes");
		String dataset = lsgi_home + (String) request.get("dataset");
		int queryServicePort = (int) request.get("queryServicePort");		
		
		System.out.println("Environment: " + serverAddress + ", Nodes: " + nodes + ", Dataset: " + dataset + ", QueryServicePort:" + queryServicePort);
						
		HashMap<String, Object> response = new HashMap<String, Object>();		
			
		//If tunneling is needed, use a different port 
		int sshPort;
		if(serverAddress.equals("localhost") || serverAddress.equals("127.0.0.1")){
			sshPort=TUNNEL_PORT;
			System.out.println("Tunneling at " + serverAddress + ":" + sshPort);
		}else{
			sshPort=SSH_DEFAULT_PORT; //Default SSH port
		}
		
		try{						
			SSHConnector ssh = new SSHConnector(); 
			ssh.init();
			Session session = ssh.connectPwdless(serverAddress, sshPort);
			String cmd = "cd "+ lsgi_home + DEMO_SCRIPTS_PATH + "; ./" + LAUNCH_SCRIPT + " " + nodes + " " + dataset + " " + queryServicePort;
			System.out.println("Executing: " + cmd);
			List<String> sshResult = ssh.executeCommand(session, cmd );
			ssh.disconnect(session);
			
			//Parse results
			Iterator<String> it = sshResult.iterator();			
			while(it.hasNext()){
				String line = it.next(); 
				if (line.contains("Done!")){
					response.put("status", "success");
					break;
				}
				response.put("status", "failed");				
			}	
			
		}catch(Exception e){
			logger.error(e);
			e.printStackTrace();			
			response.put("status", "error");		
		}
		
		return response;
	}
	
	
	/**
	 * Stop Job and QueryService in specific environment&#46;
	 * Verifies if "Done&#33;" string was returned in order to notify success state to the UI&#46;
	 * It also makes the assumption that if the environment to run is "localhost" (or 127.0.0.1) 
	 * then a tunneling process is needed to access the real destination environment 
	 * (for example, when accessing a virtual machine without public IP) 
	 * <br><br>	 
	 * <code>
	 * Example:<br>
	 * INPUT:<br>
	 * 	{"environment": "build-l4tm-2.u.labs.hpecorp.net", "lsgi_home": "/home/gomariat/graphs/code/LSGi"}<br>
	 * OUTPUT:<br>
	 * 	{"status":"success"}
	 * </code>
	 * @param request	The request object containing: environment, lsgi_home in JSON format
	 * @method stop
	 * @return {Object}	Stop results in JSON format
	*/
	@RequestMapping(value = "/stop", method = RequestMethod.POST, consumes = "application/json", produces = "application/json")
	@ResponseBody
	public HashMap<String, Object> stop(@RequestBody HashMap<String, Object> request) {	
		logger.info("Running stop");
		
		String serverAddress = (String) request.get("environment");
		String lsgi_home = (String) request.get("lsgi_home");
		System.out.println("Environment: " + serverAddress);		
		  	
		HashMap<String, Object> response = new HashMap<String, Object>();			
						
		//If tunneling is needed, use a different port
		int sshPort;
		if(serverAddress.equals("localhost") || serverAddress.equals("127.0.0.1")){
			sshPort=TUNNEL_PORT;
			System.out.println("Tunneling at " + serverAddress + ":" + sshPort);
		}else{
			sshPort=22; //Default SSH port
		}
				
		try{				
			SSHConnector ssh = new SSHConnector(); 
			ssh.init();
			Session session = ssh.connectPwdless(serverAddress, sshPort);	
			String cmd = "cd "+ lsgi_home + DEMO_SCRIPTS_PATH + "; ./" + STOP_SCRIPT;
			List<String> sshResult = ssh.executeCommand(session, cmd);
			ssh.disconnect(session);
			
			//Parse results
			Iterator<String> it = sshResult.iterator();			
			while(it.hasNext()){
				String line = it.next(); 
				if (line.contains("Done!")){
					response.put("status", "success");
					break;
				}
				response.put("status", "failed");				
			}	
			
		}catch(Exception e){
			logger.error(e);
			e.printStackTrace();			
			response.put("status", "error");		
		}
		
		return response;
	}
	
	
	
	/**
	 * Gets inference results by VertexId
	 * <br><br>	 
	 * <code>
	 * Example:<br>
	 * INPUT:<br>
	 * 	{"environment": "build-l4tm-2.u.labs.hpecorp.net", "queryServicePort":58000, "vId":0}<br>
	 * OUTPUT:<br>
	 * 	{"iterationResults":{"vId":{"vid":0,"p_0":0.863,"state":"0","iteration":1000},"stats":{"nonconverge":1,"iteration":1000,"time":224,"totalVertices":2077057,"state0":2057000,"convergence":93.2}}}
	 * </code>
	 * @param request	The request object containing: environment, queryServicePort and vertexId in JSON format
	 * @method queryByVertexId
	 * @return {Object}	Inference results and stats in JSON format
	*/
	@RequestMapping(value = "/queryByVertexId", method = RequestMethod.POST, consumes = "application/json", produces = "application/json")
	@ResponseBody
	public HashMap<String, Object> queryByVertexId(@RequestBody HashMap<String, Object> request) {	
		logger.info("Running queryByVertexId");
    	
		String serverAddress = (String) request.get("environment");		
		int queryServicePort = (int) request.get("queryServicePort");
		int vId = (int) request.get("vId");
		
		HashMap<String, Object> response = new HashMap<String, Object>();			
		
		try{					
			//Request
			QueryRequest queryRequest = new QueryRequest();
			queryRequest.type = 1; //1:queryVertexById
			queryRequest.value = vId;
			
			//Client
			ClientSocket client = new ClientSocket();
			System.out.println("Connecting to: " + serverAddress + ":" + queryServicePort);			
			client.connect(queryServicePort, serverAddress);
			client.send(queryRequest);
			byte[] results = client.receive();			
			
			Object resultsObj = SerializationUtils.deserialize(results);			
			response.put("iterationResults",resultsObj);			
			
			client.close();
			
		}catch(Exception e){
			logger.error(e);
			e.printStackTrace();			
			response.put("status", "error");			
		}
		
		return response;
	}
	

	/**
	 * Gets inference results by Probability Threshold
	 * <br><br>	 
	 * <code>
	 * Example:<br>	 
	 * INPUT:<br>
	 * 	{"environment": "build-l4tm-2.u.labs.hpecorp.net", "queryServicePort":58000, "probability":51}<br>
	 * OUTPUT:<br>
	 * 	{"iterationResults":{"topK":[{"vid":0,"p_0":0.863,"state":"0","iteration":1000},{"vid":2,"p_0":0.996,"state":"0","iteration":1000},{"vid":6,"p_0":1.0,"state":"0","iteration":1000},{"vid":7,"p_0":1.0,"state":"0","iteration":1000},{"vid":8,"p_0":1.0,"state":"0","iteration":1000}],"stats":{"nonconverge":1,"iteration":1000,"time":224,"totalVertices":2077057,"state0":2057000,"convergence":93.2}}}
	 * </code>
	 * @param request	The request object containing: environment, queryServicePort and probability in JSON format
	 * @method queryByProbThreshlod
	 * @return {Object}	Inference results and statisticss in JSON format
	*/
	@RequestMapping(value = "/queryByProbThreshold", method = RequestMethod.POST, consumes = "application/json", produces = "application/json")
	@ResponseBody
	public HashMap<String, Object> queryByProbThreshold(@RequestBody HashMap<String, Object> request) {	
		logger.info("Running queryByProbThreshold");
    	
		String serverAddress = (String) request.get("environment");		
		int queryServicePort = (int) request.get("queryServicePort");
		int probability = (int) request.get("probability");
		
		HashMap<String, Object> response = new HashMap<String, Object>();			
		
		try{			
			//Request
			QueryRequest queryRequest = new QueryRequest();
			queryRequest.type = 3; //3:doQueryByThreshold
			queryRequest.value = probability;
			
			//Client
			ClientSocket client = new ClientSocket();
			System.out.println("Connecting to: " + serverAddress + ":" + queryServicePort);
			client.connect(queryServicePort, serverAddress);
			client.send(queryRequest);
			byte[] results = client.receive();						
			
			Object resultsObj = SerializationUtils.deserialize(results);			
			response.put("iterationResults",resultsObj);					
			
			client.close();	
		}catch(Exception e){
			logger.error(e);
			e.printStackTrace();			
			response.put("status", "error");			
		}
		
		return response;
	}
	
	
	/**
	 * Gets inference results - only statistics - by Probability Threshold
	 * <br><br>	 
	 * <code>
	 * Example:<br>
	 * INPUT:<br>
	 * 	{"environment": "build-l4tm-2.u.labs.hpecorp.net", "queryServicePort":58000, "probability":51}<br>
	 * OUTPUT:<br>
	 * 	{"iterationResults":{"stats":{"nonconverge":1,"iteration":1000,"time":224,"totalVertices":2077057,"state0":2057000,"convergence":93.2}}}
	 * </code>
	 * @param request	The request object containing: environment, queryServicePort and probability in JSON format
	 * @method queryByProbThreshlod
	 * @return {Object}	Inference results - only statistics - in JSON format
	*/
	@RequestMapping(value = "/queryByProbThresholdStats", method = RequestMethod.POST, consumes = "application/json", produces = "application/json")
	@ResponseBody
	public HashMap<String, Object> queryByProbThresholdStats(@RequestBody HashMap<String, Object> request) {	
		logger.info("Running queryByProbThresholdStats");
				
		String serverAddress = (String) request.get("environment");		
		int queryServicePort = (int) request.get("queryServicePort");
		int probability = (int) request.get("probability");
    	
		HashMap<String, Object> response = new HashMap<String, Object>();			
		
		try{			
			//Request
			QueryRequest queryRequest = new QueryRequest();
			queryRequest.type = 5; //3:doQueryByThreshold, 5:doQueryByThresholdStats
			queryRequest.value = probability;
			
			//Client
			ClientSocket client = new ClientSocket();
			System.out.println("Connecting to: " + serverAddress + ":" + queryServicePort);
			client.connect(queryServicePort, serverAddress);
			client.send(queryRequest);
			byte[] results = client.receive();						
			
			Object resultsObj = SerializationUtils.deserialize(results);			
			response.put("iterationResults",resultsObj);					
			
			client.close();	
		}catch(Exception e){
			logger.error(e);
			e.printStackTrace();			
			response.put("status", "error");			
		}
		
		return response;
	}
	
	
}
