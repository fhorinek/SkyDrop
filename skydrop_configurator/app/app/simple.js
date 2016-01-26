"use strict";

app.controller("simple", ["memory", "$scope", "$q", function (memory, $scope, $q) {
	
	var deferred = $q.defer();

	deferred.promise.then(undefined, undefined, function (data){
		$scope.list = data;
	});	
	
	memory.getAllValues(deferred);
	
	$scope.list = {};
	
	$scope.$on('$destroy', function() {
		memory.remove_notify(deferred);
	});	
}]);
