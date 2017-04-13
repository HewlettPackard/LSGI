'use strict';

/**
* @module controllers
* @class LSGiViewerCtrl
* @author Janneth Rivera, Romina Espinosa
*/
lsgiWebConsole.controller('LSGiViewerCtrl', function($scope, $routeParams, $http, LSGiViewerService) {

		
	//Set default values	
	//Carousel
    $('.carousel').carousel(); //init  
    $scope.lastIdx = 0;
    $scope.lastSelectedDataset = 0;
    $scope.lastSelectedNodes = 0;
	$scope.searchType = 1;
    $scope.probability = 51;
    $scope.isLaunchBtnDisabled = false;
    $scope.isQueryBtnDisabled = false;
    //Inference
    $scope.nIterations = 100; 
    //Results section
    $scope.lastIteration = -1;
    $scope.showResults = false;
    $scope.showStopBtn = false;
    //Time bar
    $scope.timeScale= 0.2;//width = (currTime*timeScale)%
	$scope.currTime= 0;
	

	
	/**
     * @method $scope.loadExecutionEnvironments
	 * Loads execution environments options and sets default settings
	 */
	$scope.loadExecutionEnvironments = function(){
		LSGiViewerService.getExecutionConfiguration().then(function(r) {
			if(r.data){	//console.log(r.data);
				
				//Load options in UI
				$scope.environments = r.data.environments;
				$scope.datasets = r.data.datasets;
				$scope.nodes = r.data.nodes;
				
				//Set default environment
				$scope.selectEnvironment(0);				
			}
		}, function(reason){
			$scope.errorMsgText="The web services are not available. Please try again later.";
			$('#errorMsg').fadeIn();//show error msg			
			console.log("Error while loading Execution Environments. " + JSON.stringify(reason));
		});
	}
	
	/**
     * @method $scope.selectEnvironment
	 * Selects specific environment and get available dataset/nodes options
	 * Selects first dataset/nodes by default to show in menu
	 */
	$scope.selectEnvironment = function(i){
		$scope.availableDatasets = [];
		$scope.availableNodes = [];
		
		//Set selected environment
		$scope.selectedEnvironment= $scope.environments[i];
		
		//Get available datasets for selected environment
		_.each($scope.selectedEnvironment.availableDatasets, function(id){
			$scope.availableDatasets.push($scope.datasets[id]);
		
		});
			
		//Get available nodes for selected environment
		_.each($scope.selectedEnvironment.availableNodes, function(id){
			$scope.availableNodes.push($scope.nodes[id]);
		});
		
		//Set selected dataset
		$scope.selectDataset(0);
		//Set selected nodes
		$scope.selectNodes(0); 
	}
	
	/**
     * @method $scope.selectDataset
	 * Selects specific dataset
	 */
	$scope.selectDataset = function(i){		
		//Set selected dataset
		$scope.selectedDataset= $scope.availableDatasets[i];
		
		//Set pollingTime and loadingTime for selected dataset
		$scope.pollingTime = $scope.selectedDataset.pollingTime;
		$scope.loadingTime = $scope.selectedDataset.loadingTime;
		
		//Set Graph Stats values - calculate and format
		$scope.vertices = $scope.selectedDataset.statistics.vertices.toLocaleString();
		$scope.edges = $scope.selectedDataset.statistics.edges.toLocaleString();
		$scope.s0 = $scope.selectedDataset.statistics.s0.toLocaleString();
		$scope.s1 = $scope.selectedDataset.statistics.s1.toLocaleString();
		$scope.s0Percent = (($scope.selectedDataset.statistics.s0*100)/$scope.selectedDataset.statistics.vertices).toFixed(2); 
		$scope.s1Percent = (($scope.selectedDataset.statistics.s1*100)/$scope.selectedDataset.statistics.vertices).toFixed(2);	
		
	}
	
	/**
     * @method $scope.selectNodes
	 * Selects specific number of nodes
	 */
	$scope.selectNodes = function(i){	
		//Set selected number of nodes	
		$scope.selectedNodes = $scope.availableNodes[i];
	}
	
	
	/**
     * @method $scope.reset
	 * Resets layout and variables to initial settings
	 */
	$scope.reset = function(){
		console.log("reset...");	
		
		$scope.inputTab=undefined; 				//makes carousel hide
		$scope.showStopBtn = false;
		$scope.showResults = false;
		clearInterval($scope.intervalID);		//stops polling interval, if any
		$scope.intervalID = 0;
		$scope.jobStatus = "";
		$scope.iteration = 0;
		$scope.lastIteration = -1;
		$scope.showGraphStats = false;
		
		
		
		//If previous chart exists, clean it
		if($scope.convergenceChart.svg){/* @romina */			
			$scope.convergenceChart.clear();
		}
		if($scope.stateChart.svg){/* @romina */		
			$scope.stateChart.clear();
		}
		
		//Reset Inference Stats variables
		$scope.currTime= 0;
	    $scope.graphlabTime = 0;				    
	    $scope.speedup = 0;
	    $scope.convergence = 0;
	    $scope.inferencePercentage = 0;
	}
	
	 /**
     * @method $scope.launch
	 * Launch inference job for specific {environment, path, nodes, dataset, QSPort} and
	 * Query inference results
	 */
	$scope.launch= function(){			
		$scope.reset();						//set variables and UI components to initial state
		$('#errorMsg').fadeOut(); 			//hide previous error msg, if any
		$('#infoMsg').fadeOut();			//hide previous info msg, if any
		$scope.jobStatus = "Launching...";
		//$scope.showStopBtn = true;
		$scope.showGraphStats = true;
		console.log("launch...");			
		
		//Get user inputs		
		var data = {
			"environment" : $scope.selectedEnvironment.hostname,
			"lsgi_home" : $scope.selectedEnvironment.lsgi_home,
			"nodes" : $scope.selectedNodes.value,
			"dataset" : $scope.selectedDataset.value,
			"queryServicePort" : $scope.selectedDataset.queryServicePort
		}			
		console.log(data);
		
		//Execute webservice: /launch
		LSGiViewerService.launch(data).then(function(r) {
			if(!_.isEmpty(r.data)){//console.log(r.data);
				//Success
				if(r.data.status == "success"){				
					//Delay
					//Then start polling
					$scope.jobStatus = "Loading...";
					console.log("loading delay... (" + $scope.loadingTime/1000 + " sec aprox)...") 
					setTimeout(function(){ $scope.startPolling(); }, $scope.loadingTime);
				
				//Failed
				}else if(r.data.status == "failed"){ //status=failed
					$scope.reset();
					$scope.errorMsgText="The launch service failed. Please try again later.";
					$('#errorMsg').fadeIn();	
					console.log("Error while parsing results in the launch webservice.");
				
				//Error
				}else if(r.data.status == "error"){
					$scope.reset();
					$scope.errorMsgText="The launch service is not available. Please try again later.";
					$('#errorMsg').fadeIn();	
					console.log("Error while executing launch webservice.");
				}
			//Empty
			}else{
				$scope.reset();
				$scope.errorMsgText="The launch service is not available. Please try again later.";
				$('#errorMsg').fadeIn();	
				console.log("Error while executing launch webservice.");
			}
		//If webservices are not reachable
		}, function(reason){
			$scope.reset();
			$scope.errorMsgText="The web services are not available. Please try again later.";
			$('#errorMsg').fadeIn();//show error msg			
			console.log("Error while executing launch. " + JSON.stringify(reason));
		});
		
		//Start graph stats animation
		$scope.animateCounter('.number.1', 0, $scope.selectedDataset.statistics.vertices, $scope.loadingTime+3000);
		$scope.animateCounter('.number.2', 0, $scope.selectedDataset.statistics.edges, $scope.loadingTime+3000);
		$scope.animateCounter('.number.3', 0, $scope.selectedDataset.statistics.s0, $scope.loadingTime+3000);
		$scope.animateCounter('.number.4', 0, $scope.selectedDataset.statistics.s1, $scope.loadingTime+3000);		
		
	}
    
	/**
     * @method $scope.query
	 * Query inference results for specific {environment, QSPort, probability}
	 */
	$scope.query= function(){
		console.log("query...");		
		console.log("{"+ $scope.selectedEnvironment.hostname + "," + $scope.selectedDataset.queryServicePort + ","+ $scope.probability+ "}");
		
		$scope.reset();						//set variables and UI components to initial state			
		$scope.jobStatus = "Loading...";
		$('#errorMsg').fadeOut();			//hide previuos error msg, if any
		$('#infoMsg').fadeOut();			//hide previous info msg, if any
		$scope.showGraphStats = true;		//show Graph Stats


		//Start Graph stats animation
		$scope.animateCounter('.number.1', 0, $scope.selectedDataset.statistics.vertices, 2000);
		$scope.animateCounter('.number.2', 0, $scope.selectedDataset.statistics.edges, 2000);
		$scope.animateCounter('.number.3', 0, $scope.selectedDataset.statistics.s0, 2000);
		$scope.animateCounter('.number.4', 0, $scope.selectedDataset.statistics.s1, 2000);			
		
		
		//Hide Graph Stats after a few seconds
		setTimeout(function(){ $scope.$apply(function(){
			$scope.showGraphStats = false;
        }); }, 3000);
		
		
		//Start polling - wait a few seconds first just to finish the Graph Stats animation
		setTimeout(function(){ $scope.startPolling(); }, 3000);
	}
	
	/**
     * @method $scope.startPolling
	 * Starts a loop to poll inference results
	 */
	$scope.startPolling= function(){
		console.log("Start polling... (every " + (($scope.pollingTime/$scope.selectedNodes.value)/1000) + " sec aprox)...");
		
		//Poll
		$scope.poll();
		$scope.intervalID = setInterval($scope.poll, $scope.pollingTime/$scope.selectedNodes.value);
	}
	
	/**
     * @method $scope.poll
	 * Decides if continue polling or stop
	 */
	$scope.poll= function(){
		//If current iteration has not reach max number of iterations, then keep polling
		if ($scope.iteration < $scope.nIterations){					
			$scope.searchByThreshold(); 
			$scope.showStopBtn = true;
		
		//Otherwise, stop polling
		}else{			
			$scope.$apply(function(){
				clearInterval($scope.intervalID);//cancel interval
				$scope.intervalID = 0;
				console.log("stop polling");
				$scope.showStopBtn = false;	
	        });
		}		
	}	
	
	/**
	 * @method $scope.searchByThreshold
	 * Search by probability threshold
	 */
	$scope.searchByThreshold = function(){
		console.log("searching by threshold..."); 
		
		//Get user inputs		
		var data = {
			"environment" : $scope.selectedEnvironment.hostname,
			"queryServicePort" : $scope.selectedDataset.queryServicePort,
			"probability" : $scope.probability
		}
		
		//Execute webservice: /queryByProbThresholdStats
		LSGiViewerService.searchByThresholdStats(data).then(function(r){		 
			if(r.data){		
				if(!_.isEmpty(r.data.iterationResults)){ 					
					//Set current iteration
				    $scope.iteration = r.data.iterationResults.stats.iteration;
				    
				    //If iteration==0 it means algorithm is still loading data
					if($scope.iteration == 0){ 
						$scope.lastIteration = r.data.iterationResults.stats.iteration; //save this as last iteration received
						$scope.jobStatus = "Loading...";
						console.log("still loading...");
						
						$scope.infoMsgText="The Query service is still loading, please wait a moment...";
						$('#infoMsg').fadeIn();//show still loading msg
						return;
					}
				    
				    //Set Inference Stats boxes values - calculate, format
				    $scope.currTime = r.data.iterationResults.stats.time;
				    $scope.currTimeStr = $scope.timeToString($scope.currTime);
				    $scope.graphlabTime = Math.round($scope.iteration*$scope.selectedDataset.graphlabTime);
				    $scope.graphlabTimeStr = $scope.timeToString($scope.graphlabTime);
				    if($scope.graphlabTime >= 0)	$scope.speedup = ($scope.graphlabTime/$scope.currTime).toFixed(0);	
				    $scope.convergence = (r.data.iterationResults.stats.convergence).toFixed(1);
				    $scope.inferencePercentage = ((r.data.iterationResults.stats.state0*100)/r.data.iterationResults.stats.totalVertices).toFixed(1);			    
				    				
					//Hide Graph Stats, hide loading msg, show Results 					
					$scope.showGraphStats = false;	
					$('#infoMsg').fadeOut();
					$scope.showResults = true;
					
					
					//If receiving duplicate iteration, ignore
					if($scope.lastIteration == $scope.iteration)
						return;
					
					
					//Update charts
					$scope.convergenceData.push(r.data.iterationResults.stats);	
					$scope.convergenceChart.update( //ROMINA
						{iteration: r.data.iterationResults.stats.iteration},
						{convergence: r.data.iterationResults.stats.convergence}
					);
					$scope.stateChart.update(
						{iteration: r.data.iterationResults.stats.iteration},
						{state0: r.data.iterationResults.stats.state0, state1: r.data.iterationResults.stats.totalVertices - r.data.iterationResults.stats.state0}
					);
								  	    
				    //Save last iteration
				    $scope.lastIteration = r.data.iterationResults.stats.iteration;			    
				   
				//Error
				}else if(r.data.status == "error"){
					$scope.reset();
					$scope.errorMsgText="The search by threshold service is not available. Please try again later.";
					$('#errorMsg').fadeIn();	
					console.log("Error while executing search by threshold service.");
					
				//Null				
				}else{					
					$scope.reset();
					$scope.inputTab=1; //show menu					
					$scope.errorMsgText="There is no Query service running for this configuration, please launch a job first.";
					$('#errorMsg').fadeIn(); //show error msg
					console.log("Failed while executing request to QueryService.");
				}		    
			}
			
		//If webservices are not reachable
		}, function(reason){
			$scope.reset();
			$scope.errorMsgText="The web services are not available. Please try again later.";
			$('#errorMsg').fadeIn();
			console.log("Failed while executing search by threshold. " + JSON.stringify(reason));
	    });
	}
	

	/**
	 * @method $scope.stop
	 * Stop inference job in specific environment
	 */
	$scope.stop= function(){
		console.log("stop...");	
		
		//Stop polling loop
		clearInterval($scope.intervalID);//cancel interval
		$scope.intervalID = 0;		
		
		var data = {
			"environment" : $scope.selectedEnvironment.hostname,
			"lsgi_home" : $scope.selectedEnvironment.lsgi_home
		}			
		console.log(data);
		
		//Execute webservice: /stop
		LSGiViewerService.stop(data).then(function(r) {
			if(!_.isEmpty(r.data)){	//console.log(r.data);
				//Success
				if(r.data.status == "success"){
					$scope.showStopBtn = false;
					$scope.infoMsgText="The job was stopped succesfully.";
					$('#infoMsg').fadeIn();//show still loading msg
				//Failed
				}else if(r.data.status == "failed"){ 
					$scope.errorMsgText="The job was not stopped successfully. Please try again later.";
					$('#errorMsg').fadeIn();//show error msg	
					console.log("Error while parsing results in the stop webservice.");
			
				//Error
				}else if(r.data.status == "error"){
					$scope.errorMsgText="The stop service is not available. Please try again later.";
					$('#errorMsg').fadeIn();	
					console.log("Error while executing stop webservice.");
				}
			//Empty
			}else{
				$scope.reset();
				$scope.errorMsgText="The stop service is not available. Please try again later.";
				$('#errorMsg').fadeIn();	
				console.log("Error while executing stop webservice.");
			}
		//If webservices are not reachable
		}, function(reason){	
			$scope.errorMsgText="The web services are not available. Please try again later.";
			$('#errorMsg').fadeIn();
			console.log("Error while executing stop. " + JSON.stringify(reason));
		});
	}
    
    
    
	
	
	/************************
	 * CHARTS
	 ***********************/	
	
	/**
	 * Gauge Chart
	 */
	 $scope.gaugeChart = new GaugeChart("#gaugeChart", 100 );
	
	/**
	 * Convergence Chart
	 */
	 $scope.convergenceData = [];
	 $scope.convergenceChart = new BarChart( "#convergenceChart", 250 );
	 
	/**
	 * Stacked Chart
	 */
	 $scope.stateChart = new StackedChart( "#stateChart", 250 );
	

  
	 /************************
	 * UTILS
	 ***********************/
	
	/**
	 * Animate counter
	 */
	$scope.animateCounter = function (element,startCounter,endCounter,durationTime){
		$(element).each(function () {
		    $(this).prop('Counter', startCounter).animate({Counter: endCounter}, {
		        duration: durationTime,
		        easing: 'swing',
		        step: function (now) {
		        	// What todo on every count
		            $(this).text(Math.ceil(now).toLocaleString());
		        }
		    });
		});
	}
	
	/**
	 * @method $scope.timeToString
	 * @param time in seconds
	 * @return hh:mm:ss
	 */
	$scope.timeToString= function(time){
		var h = Math.floor(time / 3600);
		var m = Math.floor(time % 3600 / 60);
		var s = Math.floor(time % 3600 % 60);
		return  (h > 0 ? (h<10 ? "0" : "") + h : "00") + ":" +	//hh:
				(m > 0 ? (m<10 ? "0" : "") + m : "00") + ":" +	//mm:
				(s > 0 ? (s<10 ? "0" : "") + s : "00"); 		//ss
	}
	
    
    //Onload calls
	$scope.loadExecutionEnvironments();	

});
