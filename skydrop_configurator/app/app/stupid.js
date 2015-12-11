"use strict";

app.controller("stupid", ['$scope', 'memory', function ($scope, memory) {
	var promise = memory.getAllValues();
	$scope.list = {};

	promise.then(function (data){
		$scope.list = data;
	});
	
	//rebind all data when they change (for load cfg)
    $scope.$watch(
    	function(){return memory.data_load;},
    	function (new_val, old_val){
    		$scope.list = memory.data_holder;
    	}
    );	

	
}]);

