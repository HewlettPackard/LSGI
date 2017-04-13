'use strict';

/**
* @module controllers
* @class LSGiSettingsCtrl
* @author Janneth Rivera
*/
lsgiWebConsole.controller('LSGiSettingsCtrl', function($scope, $routeParams, $http, LSGiSettingsService) {

	$scope.showContent = false;
	
	$scope.reset= function(){
		$scope.showContent = false;
		$('#infoMsg').fadeOut();//hide info msg, if any
		$('#errorMsg').fadeOut();//hide error msg, if any
	}
	
	/**
	 * @method $scope.load
	 * Loads a configuration file to edit
	 */
	$scope.load= function(){	
		console.log("loading file: " + $scope.selectedFileName);
		
		var data =
		{
    		filename: $scope.selectedFileName
		}
		
		LSGiSettingsService.loadConfigFile(data).then(function(r) {
			if(r.data){
				if(!_.isEmpty(r.data.fileContent)){	
					
					//Get file content
					$scope.selectedFileContent = r.data.fileContent;
					
					//Show file content	parsed and indented as JSON				
					var parsedFile = JSON.parse(JSON.stringify($scope.selectedFileContent));
				    $scope.jsonFile = JSON.stringify(parsedFile, null, 2);
				    $scope.showContent = true;	
					
				}else if(r.data.status == "error"){
					$scope.errorMsgText="There was an error while loading the file. Please try again later.";
					$('#errorMsg').fadeIn();	
					console.log("Error while loading file.");
				}
			}
		}, function(reason){
			$scope.errorMsgText="The web services are not available. Please try again later.";
			$('#errorMsg').fadeIn();//show error msg			
			console.log("Error while loading configuration file. " + JSON.stringify(reason));
		});
		
	}
	
	/**
	 * @method $scope.save
	 * Saves changes in configuration file
	 */
	$scope.save= function(){
		console.log("saving file: " + $scope.selectedFileName);	
		
		$('#infoMsg').fadeOut();//hide info msg, if any
		$('#errorMsg').fadeOut();//hide error msg, if any
		
		//Verify if content is valid JSON
		if(!$scope.isJson($scope.jsonFile)) return;
		
		var data =
		{
    		filename: $scope.selectedFileName,
    		fileContent: $scope.jsonFile
		}		
		//console.log(data);
    	
		LSGiSettingsService.saveConfigFile(data).then(function(r){		 
			if(r.data){ //console.log(r.data);
				//Success
				if(r.data.status == "success"){
					$scope.infoMsgText="The file was saved succesfully.";
					$('#infoMsg').fadeIn();//show still loading msg
				//Error
				}else if(r.data.status == "error"){
					$scope.errorMsgText="There was an error while saving the file. Please try again later.";
					$('#errorMsg').fadeIn();	
					console.log("Error while saving file.");
				}
			}		
		}, function(reason){
			$scope.errorMsgText="The web services are not available. Please try again later.";
			$('#errorMsg').fadeIn();//show error msg			
			console.log("Error while saving configuration file. " + JSON.stringify(reason));
		});
	 
	}
	
	/**
	 * @method $scope.isJson
	 * Verifies if string is a valid json
	 */
	$scope.isJson= function(str){
	    try {
	        JSON.parse(str);
	    } catch (e) {
	    	$scope.errorMsgText="There was an error while parsing the file. Please verify JSON format is correct and try again later.";
			$('#errorMsg').fadeIn();	
			console.log("Error while parsing file.");
	        return false;
	    }
	    return true;
	}
});
