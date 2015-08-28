//chart cfg
Chart.defaults.global.responsive = true;
Chart.defaults.global.animation = false;

Chart.defaults.global.scaleStartValue = 0;
Chart.defaults.global.scaleStepWidth = 150;
Chart.defaults.global.scaleSteps = 10;
Chart.defaults.global.scaleOverride = true;


Chart.defaults.Line.bezierCurve = false;

app.controller("audioProfile", ['$scope', '$http', 'memory', "ChartJs", function ($scope, $http, memory, chartjs) {
	var promise = memory.getAllValues();
	
	promise.then(function (data){
		$scope.list = data;
		
		$scope.refresh();
	});
	
	//rebind all data when they change (for load cfg)
    $scope.$watch(
    	function(){return memory.data_load;},
    	function (new_val, old_val){
    		$scope.list = memory.data_holder;

    		$scope.refresh();
    	}
    );	

    $scope.refresh = function()
    {
    	var freq = [];
    	var pause = [];
    	var length = [];
    	
    	for (var i=0; i < 41; i++)
    	{
    		freq.push(memory.getActualValue("cfg_audio_profile_freq_" + i));
    		length.push(memory.getActualValue("cfg_audio_profile_length_" + i));
    		pause.push(memory.getActualValue("cfg_audio_profile_pause_" + i));
    	}
    
    	$scope.data = [freq, length, pause];  
    };
	
	$scope.labels = [];
	
	for (var i = -10 ; i <= 10; i += 0.5)
		$scope.labels.push(i + " m/s");
	
	$scope.series = ['Frequency [Hz]', 'Length [ms]', 'Pause [ms]'];
		
	$scope.data = [];
	$scope.point_drag = false;
	$scope.point_series = false;
	$scope.point_index = false;
	
	$scope.onHover = function (points, evt, chart) 
	{

		if ($scope.point_drag)
		{
			$scope.point_index = points[0].index;
			
			var name = ["cfg_audio_profile_freq_", "cfg_audio_profile_length_", "cfg_audio_profile_pause_"][$scope.point_series] + $scope.point_index;
			
			var val = memory.getActualValue(name);
			var y = evt.layerY;

			
			val = in_range(chart.scale.calculateVal(y), 0, 2000);
			
			//store to data holder
			memory.setActualValue(name, val);
			//update graph
			$scope.data[$scope.point_series][$scope.point_index] = val;    
		}
	};	
	
	$scope.onUp = function(points, evt)
	{
		$scope.point_drag = false;
	};

	$scope.onDown = function(points, evt)
	{
//	    console.log("Down", points, evt);
	    var x = evt.layerX;
	    var y = evt.layerY;
	    
	    var dist = false;
	    
	    for (var i in points)
    	{
	    	var point = points[i];
	    	
	    	var p_dist = Math.pow(point.x - x, 2) + Math.pow(point.y - y, 2); 
	    	
	    	if (p_dist < dist || !dist)
	    	{
	    		dist = p_dist;
	    		$scope.point_series = $scope.series.indexOf(point.datasetLabel);
	    		$scope.point_index = point.index;
	    		$scope.point_drag = true;
	    	}
    	
    	}
	}
}]);

