"use strict";

app.controller("varList", ["memory", "$scope", function (memory, $scope) {
	var promise = memory.getAllValues();
	
	$scope.list = {};
	
	promise.then(function (data){
		$scope.list = data;
	});
	
	//rebind all data when they change (for load cfg)
    $scope.$watch(
    	function(){return memory.data_load;},
    	function (new_val, old_val){$scope.list = memory.data_holder;}
    );
    
}]);

