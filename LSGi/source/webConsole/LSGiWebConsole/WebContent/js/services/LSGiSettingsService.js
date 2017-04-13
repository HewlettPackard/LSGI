'use strict';

/**
* @module services
* @author Janneth Rivera
*/
angular.module('lsgiWebConsole').factory('LSGiSettingsService', function($http, $rootScope) {
	var service = {};
	

	service.loadConfigFile = function(data) {
		return $http({
			url : $rootScope.serverURL + '/settings/load',
			method : "POST",
			headers: {'Content-Type': 'application/json'},
			data: JSON.stringify(data)
		}).then(function(res) {
			return res;
		});
	};
	
	service.saveConfigFile = function(data) {
		return $http({
			url : $rootScope.serverURL + '/settings/save',
			method : "POST",
			headers: {'Content-Type': 'application/json'},
			data: JSON.stringify(data)
		}).then(function(res) {
			return res;
		});
	};

	return service;
});