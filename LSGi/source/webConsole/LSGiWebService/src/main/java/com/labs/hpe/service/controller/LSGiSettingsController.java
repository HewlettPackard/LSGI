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

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.URL;
import java.util.HashMap;
import java.util.List;
import java.util.Properties;

import javax.annotation.PostConstruct;

import org.apache.log4j.Logger;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.type.TypeReference;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * This class contains web services for LSGiWebConsole Settings
 * @class LSGiSettingsController
 * @author Janneth Rivera, Tere Gonzalez
 * September 9, 2016
 */

@Controller
@RequestMapping("/settings")
public class LSGiSettingsController {

	private static final Logger logger = Logger.getLogger(LSGiSettingsController.class);	
	
		
	@PostConstruct
	public void init() {
		logger.info("Init LSGi Settings controller");	
		
	}
	
	
	/**
	 * Loads a configuration file content
	 * <br><br>	 
	 * <code>
	 * Example:<br>
	 * INPUT:<br>
	 * 	{"filename": "environments.json"}<br>
	 * OUTPUT:<br>
	 * 	{"environments": [{"id": 2,"name": "Single Node-L4TM","hostname": "build-l4tm-2.u.labs.hpecorp.net","image": "Servers.png","lsgi_home": "/home/gomariat/graphs/code/LSGi","description": "The l4tm-selfhosted-2 is a single node (DL580) running L4TM and librarian self hosted version&#46;","availableDatasets": [0,1,2,3,4],"availableNodes": [0,1,2,3,4,5,6]}]}
	 * </code>
	 * @method loadFile
	 * @return {Object}	Configuration file content in JSON format
	*/		
	@RequestMapping(value = "/load", method = RequestMethod.POST, consumes = "application/json", produces = "application/json")
	@ResponseBody
	public HashMap<String, Object> loadFile(@RequestBody HashMap<String, Object> request) {	
		logger.info("Running loadFile");		
	  	
		HashMap<String, Object> response = new HashMap<String, Object>();			
				
		try{	
			//Read file
			String filename = (String) request.get("filename");			
			String file = "properties/" + filename;
			InputStream in = getClass().getClassLoader().getResourceAsStream(file);
			
			//Convert to JSON obj
			ObjectMapper mapper = new ObjectMapper();			
			TypeReference<List<HashMap<String,Object>>> typeRef = new TypeReference<List<HashMap<String,Object>>>() {};			
		    List<HashMap<String,Object>> obj1 = mapper.readValue(in, typeRef);
		    //System.out.println(obj1); 		    
			
		    logger.info("Succesfully load file.");
			response.put("fileContent", obj1);
			
			in.close();
		}catch(Exception e){
			logger.error("Error while loading file. ", e);
 			response.put("status", "error");	
		}
		
		return response;
	}
	
	/**
     * Saves changes in configuration file
     * <br><br>	 
	 * <code>
	 * Example:<br>
	 * INPUT:<br>
	 * 	{"filename": "environments.json", "content": [{"id": 1,"name": "Single Node-L4TM","hostname": "build-l4tm-2.u.labs.hpecorp.net","image": "Servers.png","lsgi_home": "/home/gomariat/graphs/code/LSGi","description": "The l4tm-selfhosted-2 is a single node (DL580) running L4TM and librarian self hosted version&#46;","availableDatasets": [0,1,2,3,4],"availableNodes": [0,1,2,3,4,5,6]}]}<br>
	 * OUTPUT:<br>
	 * 	{"status":"success"}
	 * </code>
	 * @param request	The request object containing: filename and fileContent in JSON format
	 * @method saveConfigFile
	 * @return {Object}	Save results in JSON format
	*/
     @RequestMapping(value = "/save", method = RequestMethod.POST, consumes = "application/json", produces = "application/json")
     @ResponseBody
     public HashMap<String, Object> saveFile(@RequestBody HashMap<String, Object> request) {
    	 logger.info("Running saveConfigFile");    	 
    	 
    	 HashMap<String, Object> response = new HashMap<String, Object>();	
    	
 		 		
 		try{
 			String filename = (String) request.get("filename");
 	    	String fileContent = (String) request.get("fileContent");    	 
 	    	System.out.println("Saving file: " + filename);	  	    	 
			
 	    	//Get file
			URL url = getClass().getClassLoader().getResource("properties/" + filename);
 			File file = new File(url.toURI().getPath()); 			
 			FileOutputStream fos = new FileOutputStream(file);
 			
 			//If file doesn't exists, then create it
			if (!file.exists()) {
				file.createNewFile();
			}
			
			//Get the content in bytes
			byte[] contentInBytes = fileContent.getBytes();

			//Write to file
			fos.write(contentInBytes);
			fos.flush();
			fos.close();
			
			logger.info("Succesfully write to file.");
	 		response.put("status", "success");
			
 		}catch(Exception e){
 			logger.error("Error while writing to file. ", e);
 			response.put("status", "error");
 		} 		
    	
 		return response;
     }
     
}