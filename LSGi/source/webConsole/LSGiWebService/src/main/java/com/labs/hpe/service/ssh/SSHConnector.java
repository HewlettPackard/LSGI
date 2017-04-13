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

package com.labs.hpe.service.ssh;


import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import com.jcraft.jsch.Channel;
import com.jcraft.jsch.ChannelExec;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.Session;


/**
 * This class contains several methods to connect through SSH
 * @author Janneth Rivera
 * March 3, 2016
 */
public class SSHConnector {

	private String user;
	private String privateKey;
	
	/**
	 * Initializes variables from configuration file
	 */
	public void init(){
		try {			
			//Read properties from file		
			Properties prop = new Properties();		
			String file = "properties/ssh.properties";			
			InputStream input = getClass().getClassLoader().getResourceAsStream(file);
			prop.load(input);	
			
    		user = prop.getProperty("user"); 
    		privateKey = prop.getProperty("privateKey");
			
			input.close();
			//System.out.println(user);
			//System.out.println(privateKey);
			//System.out.println(host);
			//System.out.println(cmd);
    	} catch (IOException ex) {
    		System.out.println(ex);
        }			
	}
	
	
	/**
	 * SSH Connect password-less
	 * @param host	The hostname or IP address to connect
	 * @param port	The port to connect
	 * @return	{Object}	The created session
	 */
	public Session connectPwdless(String host, int port){
		Session session = null;
		try{            
            java.util.Properties config = new java.util.Properties(); 
            config.put("StrictHostKeyChecking", "no");
            JSch jsch = new JSch();            
            
        
            jsch.addIdentity(
                    user,    						// String userName
                    privateKey.getBytes(),          // byte[] privateKey 
                    null,            				// byte[] publicKey
                    null  							// byte[] passPhrase
                );
            
            System.out.println("identity added for user: " + user);
            
            session = jsch.getSession(user, host, port);     
            session.setConfig(config);
            session.connect();
            System.out.println("Connected");            
            
		}catch(Exception e){
            e.printStackTrace();
        }
		
		return session;
	}
	
	/**
	 * Close SSH connection
	 * @param session	The session to close
	 */
	public void disconnect(Session session){
		session.disconnect();
		System.out.println("Disconnected");
	}
	
	/**
	 * Execute a command in SSH shell
	 * @param session	The session to execute commands
	 * @param cmd	The command to execute
	 * @return	{List[String]}	The results of the executed command as a list
	 */
	public List<String> executeCommand(Session session, String cmd){
		List<String> results = new ArrayList<String>();
		
		try{
			Channel channel=session.openChannel("exec");
            ((ChannelExec)channel).setCommand(cmd);
            
            BufferedReader in=new BufferedReader(new InputStreamReader(channel.getInputStream()));
            channel.connect();
           
            while(true){
            	String res = null;
            	while((res = in.readLine())!=null)  results.add(res);
            		   
            	if(channel.isClosed()){
            		//System.out.println("exit-status: "+channel.getExitStatus());
                break;
              }
              try{Thread.sleep(1000);}catch(Exception ee){}
            }
            channel.disconnect();
			
		}catch(Exception e){
			e.printStackTrace();
		}
		
		System.out.println(results);
		return results;
	}
	
	
	/**
	 * Transfer file from local to remote via SSH shell connection
	 * @param session	The session to use
	 * @param filename	The file name to transfer
	 * @param fromPath	The source path to the file
	 * @param toPath	The destination path of the file
	 * 
	 */
	public void fileTransfer(Session session, String filename, String fromPath, String toPath){
		FileInputStream fis=null;
		try{ 
			
			boolean ptimestamp = true;
		 
			// exec 'scp -t rfile' remotely
			String command = "sudo scp " + (ptimestamp ? "-p" :"") +" -t "+ toPath+filename;
			Channel channel = session.openChannel("exec");
			((ChannelExec)channel).setCommand(command);
			
			 
			// get I/O streams for remote scp
			OutputStream out = channel.getOutputStream();
			InputStream in = channel.getInputStream();
			
			channel.connect();
			
			if(checkAck(in)!=0){
				System.exit(0);
			}
	
			File _lfile = new File(fromPath + "/" + filename);
			 
			if(ptimestamp){
				command="T "+(_lfile.lastModified()/1000)+" 0";
				// The access time should be sent here,
				// but it is not accessible with JavaAPI ;-<
				command+=(" "+(_lfile.lastModified()/1000)+" 0\n"); 
				out.write(command.getBytes()); out.flush();
				if(checkAck(in)!=0){
					System.exit(0);
				}
			}
			 
			// send "C0644 filesize filename", where filename should not include '/'
			long filesize=_lfile.length();
			command="C0644 "+filesize+" ";
			if(filename.lastIndexOf('/')>0){
				command+=filename.substring(filename.lastIndexOf('/')+1);
			}
			else{
				command+=filename;
			}
			command+="\n";
			out.write(command.getBytes()); out.flush();
			
			if(checkAck(in)!=0){
				System.exit(0);
			}
			 
			// send a content of filename
			fis=new FileInputStream(fromPath + "/" + filename);
			byte[] buf=new byte[1024];
			while(true){
				int len=fis.read(buf, 0, buf.length);
				if(len<=0) break;
					out.write(buf, 0, len); //out.flush();
			}
			fis.close();
			fis=null;
			// send '\0'
			buf[0]=0; out.write(buf, 0, 1); out.flush();
			if(checkAck(in)!=0){
				System.exit(0);
			}
			out.close();
			 
			channel.disconnect();
			System.out.println("File: " + filename + " was succesfully transfered.");
			
		}
	    catch(Exception e){
	      System.out.println(e);
	      try{if(fis!=null)fis.close();}catch(Exception ee){}
	    }		
	}
	
	
	public int checkAck(InputStream in) throws IOException{
	    int b=in.read();
	    // b may be 0 for success,
	    //          1 for error,
	    //          2 for fatal error,
	    //          -1
	    if(b==0) return b;
	    if(b==-1) return b;
	 
	    if(b==1 || b==2){
	    	StringBuffer sb=new StringBuffer();
	    	int c;
	    	do {
	    		c=in.read();
	    		sb.append((char)c);
	    	}
	    	while(c!='\n');
    		if(b==1){ // error
    			System.out.print(sb.toString());
    		}
    		if(b==2){ // fatal error
    			System.out.print(sb.toString());
    		}
	    }
	    return b;
	  }
	

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		String serverAddress="mercado-3.hpl.hp.com"; //hostname or ip
		String cmd="ls -l";
		
		SSHConnector ssh = new SSHConnector();			
		ssh.init();
		Session session = ssh.connectPwdless(serverAddress,22);		
		ssh.executeCommand(session, cmd);
		ssh.disconnect(session);		
		
	}

	
}

