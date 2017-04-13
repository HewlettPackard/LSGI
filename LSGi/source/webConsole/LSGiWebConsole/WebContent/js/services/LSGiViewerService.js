'use strict';

/**
* @module services
* @author Janneth Rivera
*/
angular.module('lsgiWebConsole').factory('LSGiViewerService', function($http, $rootScope) {
	var service = {};
	
	service.getExecutionConfiguration = function() {
		return $http({
			url : $rootScope.serverURL + '/theMachine/getExecutionConfiguration',
			method : "GET"
		}).then(function(res) {
			return res;
		});
	};
	
	service.launch = function(data) {
		return $http({
			url : $rootScope.serverURL + '/theMachine/launch',
			method : "POST",
			headers: {'Content-Type': 'application/json'},
			data: JSON.stringify(data)
		}).then(function(res) {
			return res;
		});
	};
	
	service.stop = function(data) {
		return $http({
			url : $rootScope.serverURL + '/theMachine/stop',
			method : "POST",
			headers: {'Content-Type': 'application/json'},
			data: JSON.stringify(data)
		}).then(function(res) {
			return res;
		});
	};
	
	
	service.getJobStatus = function(environment) {
		return $http({
			url : $rootScope.serverURL + '/theMachine/getJobStatus/' + environment + '/',
			method : "GET"
		}).then(function(res) {
			return res;
		});
	};
	
	service.searchById = function(data) {
		return $http({
			url : $rootScope.serverURL + '/theMachine/queryByVertexId',
			method : "POST",
			headers: {'Content-Type': 'application/json'},
			data: JSON.stringify(data)
		}).then(function(res) {
			return res;
		});
	};
			
	service.searchByThreshold = function(data) {
		return $http({
			url : $rootScope.serverURL + '/theMachine/queryByProbThreshold',
			method : "POST",
			headers: {'Content-Type': 'application/json'},
			data: JSON.stringify(data)
		}).then(function(res) {
			return res;
		});
	};
	
	service.searchByThresholdStats = function(data) {
		return $http({
			url : $rootScope.serverURL + '/theMachine/queryByProbThresholdStats',
			method : "POST",
			headers: {'Content-Type': 'application/json'},
			data: JSON.stringify(data)
		}).then(function(res) {
			return res;
		});
	};

	return service;
});