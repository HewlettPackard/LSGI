'use strict';

// Declare app level module which depends on filters, and services
$(document).ready(function() {
	angular.bootstrap(document, [ 'lsgiWebConsole' ]);
});


var lsgiWebConsole = angular.module('lsgiWebConsole',
		[]).config(
		function($routeProvider, $httpProvider) {
						
			$routeProvider.when('/', {
				templateUrl : 'partials/LSGiViewerView.html',
				controller : 'LSGiViewerCtrl'
			});
			
			$routeProvider.when('/settings', {
				templateUrl : 'partials/LSGiSettingsView.html',
				controller : 'LSGiSettingsCtrl'
			});
			
			$routeProvider.when('/about', {
				templateUrl : 'partials/LSGiAboutView.html'/*,
				controller : 'LSGiViewerCtrl'*/
			});
				
			$routeProvider.otherwise({
				redirectTo : '/'
			});

			

		});

// Function to declare all the environment/global variables. This runs before
// everything else.
lsgiWebConsole
		.run(function($rootScope) {

			//Production server Name:			
			//$rootScope.serverURL = 'http://mercoop-26.hpl.hp.com/LSGiWebService';
			
			//Development server:
			$rootScope.serverURL = 'http://localhost:8080/LSGiWebService';
			

		});
