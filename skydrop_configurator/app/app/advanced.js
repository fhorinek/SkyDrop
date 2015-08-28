"use strict";

app.controller("advanced", ["memory", "$scope", function (memory, $scope) {
    
	var promise = memory.getAllValues();
	
	promise.then(function (data){
		$scope.list = data;
	});
	
	//rebind all data when they change (for load cfg)
    $scope.$watch(
    	function(){return memory.data_load;},
    	function (new_val, old_val){$scope.list = memory.data_holder;}
    );
    
    $scope.get_macro_value = function(macro_name)
    {
        return memory.getMacroValue(macro_name);
    }
   
    $scope.flags_checked = function(value, macro_name)
    {
        if (memory.getMacroValue(macro_name) & value)
            return true;
        else
            return false;
    }    
    
    $scope.flags_change = function(item, macro_name)
    {
        console.log(item);
        var m_value = memory.getMacroValue(macro_name);
        if (m_value & item.value)
        {
            //checked
            item.value &= ~m_value;
        }
        else
        {
            //unchecked
            item.value |= m_value;
        }
        console.log(item);
        console.log("---");
    }
    
    $scope.select_get_options = function(options)
    {
        var arr = new Array();
        
        for (var i in options)
        {
            var line = {
                "key": memory.getMacroValue(options[i][0]),
                "value": options[i][1]
            };
            arr.push(line);
        }

        return arr;
    }
    
}]);

